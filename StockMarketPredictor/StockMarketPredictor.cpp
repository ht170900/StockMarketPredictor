#include "pch.h"
#include "StockMarketPredictorDlg.h"
#include "StockMarketPredictor.h"
#include "afxdialogex.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include "gnuplot-iostream.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;
BEGIN_MESSAGE_MAP(StockMarketDialog, CWinApp)
    ON_CBN_SELCHANGE(IDC_COMBO_STOCKS, &StockMarketDialog::OnStockSelected)
END_MESSAGE_MAP()
//StockMarketDialog::StockMarketDialog(CWnd* pParent) : CWinApp(StockMarketDialog::IDD, pParent) {}
StockMarketDialog theApp;

// Application startup
StockMarketDialog::StockMarketDialog() {}

BOOL StockMarketDialog::InitInstance() {
    CWinApp::InitInstance();

    // Enable Windows XP visual styles
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    AfxEnableControlContainer();
    AfxMessageBox(_T("Starting Application..."));
    // Show the main dialog
    CStockMarketPredictorDlg dlg;
    m_pMainWnd = &dlg;
        AfxMessageBox(_T("Opening Dialog..."));
        INT_PTR nResponse = dlg.DoModal();

        if (nResponse == IDOK) {
            AfxMessageBox(_T("Dialog closed with OK."));
        }
        else if (nResponse == IDCANCEL) {
            AfxMessageBox(_T("Dialog closed with Cancel."));
        }
        else {
            AfxMessageBox(_T("Dialog failed to open!"));
        }

    return FALSE; // Exit when the dialog closes
}
//void StockMarketDialog::DoDataExchange(CDataExchange* pDX) {
//    CDialogEx::DoDataExchange(pDX);
//    DDX_Control(pDX, IDC_COMBO_STOCKS, stockCombo);
//    DDX_Control(pDX, IDC_BUTTON_ANALYZE, analyzeButton);
//}
//
//BOOL StockMarketDialog::OnInitDialog() {
//    CDialogEx::OnInitDialog();
//
//    // Populate dropdown with stock symbols
//    stockCombo.AddString(_T("AAPL"));
//    stockCombo.AddString(_T("GOOGL"));
//    stockCombo.AddString(_T("MSFT"));
//
//    return TRUE;
//}

// Helper function to fetch stock data


// Function to fetch stock data
std::vector<std::pair<int, double>> StockMarketDialog::fetchStockData(const string& symbol) {
    string apiKey = "VWNJIJ7SEPBMMLRH";
    string url = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=IBM&apikey=" + apiKey;

    CURL* curl = curl_easy_init();
    string response;
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
            double closePrice = stod(values["4. close"].get<string>());
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
VectorXd StockMarketDialog::linearRegression(const MatrixXd& X, const VectorXd& y) {
    return (X.transpose() * X).ldlt().solve(X.transpose() * y);
}

// Function to plot stock prediction
void StockMarketDialog::plotStockPrediction(const std::vector<std::pair<int, double>>& stockData, const VectorXd& theta) {
    int n = stockData.size();
    std::vector<double> days, prices;

    // Loop to extract days and prices
    for (const auto& data : stockData) {
        days.push_back(data.first);
        prices.push_back(data.second);
    }

    // Prepare for future predictions
    std::vector<double> futureDays, predictedPrices;
    for (int i = n + 1; i <= n + 5; ++i) {
        futureDays.push_back(i);
        predictedPrices.push_back(theta(0) + theta(1) * i);  // Linear prediction
    }

    // Gnuplot for plotting
    Gnuplot gp;
    gp << "set terminal png size 800,600\n";
    gp << "set output 'stock_prediction.png'\n";
    gp << "set title 'Stock Market Prediction'\n";
    gp << "set xlabel 'Days'\n";
    gp << "set ylabel 'Stock Price'\n";
    gp << "plot '-' with points title 'Actual Prices', '-' with lines title 'Predicted Prices'\n";

    // Send the actual data
    gp.send1d(std::make_pair(days, prices));

    // Send the predicted data
    gp.send1d(std::make_pair(futureDays, predictedPrices));
}

// Event handler: Fetch & Analyze Data on Selection
void StockMarketDialog::OnStockSelected() {
    int index = stockCombo.GetCurSel();
    if (index == CB_ERR) {
        AfxMessageBox(_T("Please select a stock."));
        return;
    }

    CString selectedStock;
    stockCombo.GetLBText(index, selectedStock);
    string symbol = CT2A(selectedStock);

    AfxMessageBox(_T("Fetching data..."));
    std::vector<std::pair<int, double>>  stockData = fetchStockData(symbol);
    if (stockData.empty()) {
        AfxMessageBox(_T("No data found."));
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
}