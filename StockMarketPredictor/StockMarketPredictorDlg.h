#ifndef STOCKMARKETPREDICTORDLG_H
#define STOCKMARKETPREDICTORDLG_H

#include <afxwin.h>
#include "resource.h"
#include "pch.h"
#include "afxwin.h"  // MFC Core
#include <vector>     // Required for std::vector
#include <string>     // Required for std::string
#include <Eigen/Dense> // Required for MatrixXd and VectorXd
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include "gnuplot-iostream.h"
#include "resource.h"
#include "framework.h"
#include "afxdialogex.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <string>
#include <utility>  // For std::pair
#include <string>
#include "StockMarketPredictor.h"
class CStockMarketPredictorDlg : public CDialogEx {
public:
    CStockMarketPredictorDlg(CWnd* pParent = nullptr);
    virtual ~CStockMarketPredictorDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = ID_Main};
#endif

protected:
    // In StockMarketPredictor.h
   

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnAnalyzeClicked();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnStockSelected(); // Event handler for stock selection
    
    std::vector<std::pair<int, double>> fetchStockData(const std::string& symbol);
    Eigen::VectorXd linearRegression(const Eigen::MatrixXd& X, const Eigen::VectorXd& y);
    void plotStockPrediction(const std::vector<std::pair<int, double>>& stockData, const Eigen::VectorXd& theta);
    DECLARE_MESSAGE_MAP()

private:
    CComboBox stockCombo;  // 🔹 Added declaration for combo box
    CButton analyzeButton; // 🔹 Added declaration for analyze button
    HICON m_hIcon;
};
extern size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
#endif // STOCKMARKETPREDICTORDLG_H
