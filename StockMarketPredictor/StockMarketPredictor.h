#ifndef STOCKMARKETDIALOG_H
#define STOCKMARKETDIALOG_H


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

using namespace std;
using namespace Eigen;

class StockMarketDialog : public CWinApp {
public:
    StockMarketDialog();
    
    virtual BOOL InitInstance();
    enum { IDD = ID_Main };

protected:

    afx_msg void OnStockSelected();

    DECLARE_MESSAGE_MAP()

private:
    CComboBox stockCombo;
    CButton analyzeButton;

    std::vector<std::pair<int, double>> fetchStockData(const string& symbol);
    VectorXd linearRegression(const MatrixXd& X, const VectorXd& y);
    void plotStockPrediction(const std::vector<std::pair<int, double>>& stockData, const VectorXd& theta);
};
extern StockMarketDialog theApp;


#endif // STOCKMARKETDIALOG_H
