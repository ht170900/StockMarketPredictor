reset
set title 'Stock Market Prediction'
set xlabel 'Days'
set ylabel 'Stock Price'
set grid
plot 'actual_stock_data.txt' with points title 'Actual Prices', 'predicted_stock_data.txt' with lines title 'Predicted Prices'
pause mouse close
