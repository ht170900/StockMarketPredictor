#pragma warning(disable: 26495)
#pragma warning(push)
#pragma warning(disable: 26812)
#pragma warning(pop)
#pragma warning(disable: 26451)

#include "pch.h"
#include "afxwin.h"  // MFC Core
#include <vector>     // Required for std::vector
#include <string>     // Required for std::string
#include <Eigen/Dense> // Required for MatrixXd and VectorXd
#include <iostream>
#include <windows.h>
#include <sstream>
#include <curl/curl.h>
#include "gnuplot-iostream.h"
#include "resource.h"
#include "framework.h"
#include <string>    // For std::string
#include <iostream>  // For debugging purposes (optional)
#include "StockMarketPredictorDlg.h"
#include "afxdialogex.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <utility>  // For std::pair
#include <string>
#include <filesystem>
#include <tchar.h>

namespace fs = std::filesystem;
using json = nlohmann::json;
using Eigen::MatrixXd;
using Eigen::VectorXd;
Eigen::MatrixXd matrix = Eigen::MatrixXd::Zero(3, 3);
Eigen::VectorXd vector = Eigen::VectorXd::Zero(3);
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx {
public:
    CAboutDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()
};
BEGIN_MESSAGE_MAP(CStockMarketPredictorDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_ANALYZE, &CStockMarketPredictorDlg::OnAnalyzeClicked)
    ON_CBN_SELCHANGE(IDC_COMBO_STOCKS, &CStockMarketPredictorDlg::OnStockSelected)
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}
size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    ((std::string*)userdata)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()
std::vector<std::string> fetchStockSymbols() {
    std::string apiKey = "VWNJIJ7SEPBMMLRH";
    std::string url = "https://api.example.com/get_stock_list?apikey=" + apiKey;  // Replace with actual API

    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            AfxMessageBox(_T("Failed to fetch stock symbols."));
            return {};
        }
    }

    json jsonData;
    try {
        jsonData = json::parse(response);
    }
    catch (...) {
        AfxMessageBox(_T("Error parsing stock symbols response."));
        return {};
    }

    std::vector<std::string> stockSymbols;
    for (auto& stock : jsonData["stocks"]) { // Adjust based on API response format
        stockSymbols.push_back(stock["symbol"].get<std::string>());
    }

    return stockSymbols;
}

void OpenGnuplotThread() {
    std::thread([] {
        system("start gnuplot -persist -e \"plot sin(x)\"");
        }).detach();
}
// CStockMarketPredictorDlg dialog
CStockMarketPredictorDlg::CStockMarketPredictorDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(ID_Main, pParent) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    
}

CStockMarketPredictorDlg::~CStockMarketPredictorDlg() {
    // Destructor (if needed)
}
void CStockMarketPredictorDlg::OnAnalyzeClicked() {
    AfxMessageBox(_T("Analyze button clicked! Fetching stock data..."));

    int index = stockCombo.GetCurSel();
    if (index == CB_ERR) {
        AfxMessageBox(_T("Please select a stock."));
        return;
    }

    CString selectedStock;
    stockCombo.GetLBText(index, selectedStock);
    std::string symbol = CT2A(selectedStock);

    AfxMessageBox(_T("Fetching data for ") + selectedStock);

    // Fetch stock data
    std::vector<std::pair<int, double>> stockData = fetchStockData(symbol);
    if (stockData.empty()) {
        AfxMessageBox(_T("No stock data found or failed to fetch."));
        return;
    }

    // Prepare data for ML
    int n = stockData.size();
    MatrixXd X(n, 2);
    VectorXd y(n);
    for (int i = 0; i < n; ++i) {
        X(i, 0) = 1;  // Bias term
        X(i, 1) = stockData[i].first;
        y(i) = stockData[i].second;
    }

    // Perform Linear Regression
    VectorXd theta = linearRegression(X, y);

    // Plot Predictions
    plotStockPrediction(stockData, theta);

    AfxMessageBox(_T("Prediction saved to stock_prediction.png"));

    // Force update UI
    Invalidate();
    UpdateWindow();
}

std::vector<std::pair<int, double>> CStockMarketPredictorDlg::fetchStockData(const std::string& symbol) {
    std::string apiKey = "VWNJIJ7SEPBMMLRH";
    std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=IBM&apikey=" + apiKey;

    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            AfxMessageBox(_T("Failed to fetch data."));
            return {};
        }
    }

    // Parse JSON
    json jsonData;
    try {
        jsonData = json::parse(response);
    }
    catch (...) {
        AfxMessageBox(_T("Error parsing JSON response."));
        return {};
    }

    // Extract "Time Series (Daily)"
    if (!jsonData.contains("Time Series (Daily)")) {
        AfxMessageBox(_T("Invalid response from API."));
        return {};
    }

    std::vector<std::pair<int, double>> stockData;
    int day = 1;
    for (auto& [date, values] : jsonData["Time Series (Daily)"].items()) {
        try {
            double closePrice = stod(values["4. close"].get<std::string>());
            stockData.push_back({ day++, closePrice });
        }
        catch (...) {
            AfxMessageBox(_T("Error extracting stock data."));
            return {};
        }
    }

    return stockData;
}

// Function to perform linear regression
VectorXd CStockMarketPredictorDlg::linearRegression(const MatrixXd& X, const VectorXd& y) {
    return (X.transpose() * X).ldlt().solve(X.transpose() * y);
}

// Function to plot stock prediction
void CStockMarketPredictorDlg::plotStockPrediction(const std::vector<std::pair<int, double>>& stockData, const VectorXd& theta) {
    std::thread t([stockData, theta] {
        int n = stockData.size();
        std::string actualDataFile = "actual_stock_data.txt";
        std::string predictedDataFile = "predicted_stock_data.txt";
        std::string gnuplotScriptFile = "plot_script.gnuplot";

        // Save Actual Data
        std::ofstream actualFile(actualDataFile);
        for (const auto& data : stockData) {
            actualFile << data.first << " " << data.second << "\n";
        }
        actualFile.close();

        // Save Predicted Data
        std::ofstream predictedFile(predictedDataFile);
        for (int i = n + 1; i <= n + 5; ++i) {
            predictedFile << i << " " << (theta(0) + theta(1) * i) << "\n";
        }
        predictedFile.close();

        // Create Gnuplot script
        std::ofstream scriptFile(gnuplotScriptFile);
        scriptFile << "reset\n";
        scriptFile << "set title 'Stock Market Prediction'\n";
        scriptFile << "set xlabel 'Days'\n";
        scriptFile << "set ylabel 'Stock Price'\n";
        scriptFile << "set grid\n";
        scriptFile << "plot '" << actualDataFile << "' with points title 'Actual Prices', '" << predictedDataFile << "' with lines title 'Predicted Prices'\n";

        // ✅ Fix: Use 'pause mouse close' to close terminal automatically
        scriptFile << "pause mouse close\n";

        scriptFile.close();

        // Run Gnuplot
        std::string command = "start gnuplot " + gnuplotScriptFile;
        system(command.c_str());
        });

    t.detach();  // Run thread independently
}



void CStockMarketPredictorDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_STOCKS, stockCombo);   // 🔹 Properly binds combo box
    DDX_Control(pDX, IDC_BUTTON_ANALYZE, analyzeButton); // 🔹 Properly binds button
}



BOOL CStockMarketPredictorDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    analyzeButton.EnableWindow(TRUE);
    
    //// Populate dropdown with stock symbols
    //stockCombo.AddString(_T("AAPL"));
    //stockCombo.AddString(_T("GOOGL"));
    //stockCombo.AddString(_T("MSFT"));
    std::vector<std::string> stockSymbols = fetchStockSymbols();
    for (const auto& symbol : stockSymbols) {
        stockCombo.AddString(CString(symbol.c_str()));
    }

    
    return TRUE;
}

void CStockMarketPredictorDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

void CStockMarketPredictorDlg::OnStockSelected() {
    int index = stockCombo.GetCurSel();
    if (index == CB_ERR) {
        AfxMessageBox(_T("Please select a stock."));
        return;
    }

    CString selectedStock;
    stockCombo.GetLBText(index, selectedStock);
    std::string symbol = CT2A(selectedStock);
}

void CStockMarketPredictorDlg::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    }
    else {
        CDialogEx::OnPaint();
    }
}

HCURSOR CStockMarketPredictorDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}


