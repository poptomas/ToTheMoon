import datetime
import requests
import argparse
import pandas as pd
import warnings
import json
import os
import matplotlib.pyplot as plt

# Do not spam warnings to console
warnings.filterwarnings("ignore")

show_graph = False

def flatten(list):
    if list == []:
        return []
    return [item for sublist in list for item in sublist]

def log_success(name):
    print("Initial indicator values for {0} is ready".format(name))

def log_failure(name):
    print("Initial indicator values for {0} was not found".format(name))

def remove_extension(fname):
    return fname[:fname.index(".")]

def clean_dataset(filename):
    dataset = pd.DataFrame()
    dataset = fill_with_latest_data(dataset, filename)
    dataset = feature_engineering(dataset)
    #dataset = dataset.tail(100)
    # remove malformed rows due to feature engineering (needs previous values which end up being NaN)
    try:
        dataset.to_csv(filename, index=False)
        log_success(filename)
    except:
        log_failure(filename)
        return
    return dataset

def fill_with_latest_data(dataset, fname_ext):
    fname = remove_extension(fname_ext)
    base_url = "https://api.binance.com/api/v3/klines?symbol={}&interval=1m".format(fname)
    unix_idx = 0
    close_idx = 4 # indices taken from https://binance-docs.github.io/apidocs/spot/en/#kline-candlestick-data
    response = requests.get(base_url, verify=False)
    response_dict = json.loads(response.text)
    for record in response_dict:
        row = {"unix" : record[unix_idx], "close" : float(record[close_idx]) }
        dataset = dataset.append(row, ignore_index=True)
    return dataset


def set_RSI(new_dataset, period):
    new_dataset['Diff'] = new_dataset['close'].transform(lambda x: x.diff())
    new_dataset['Up'] = new_dataset['Diff']
    new_dataset.loc[(new_dataset['Up'] < 0), 'Up'] = 0

    new_dataset['Down'] = new_dataset['Diff']
    new_dataset.loc[(new_dataset['Down'] > 0), 'Down'] = 0 
    new_dataset['Down'] = abs(new_dataset['Down'])

    new_dataset['avg_up'] = new_dataset['Up'].transform(lambda x: x.rolling(window=period).mean())
    new_dataset['avg_down'] = new_dataset['Down'].transform(lambda x: x.rolling(window=period).mean())
    new_dataset['RS'] = new_dataset['avg_up'] / new_dataset['avg_down']
    new_dataset['RSI'] = 100 - (100 / (1 + new_dataset['RS']))
   
    print(new_dataset.iloc[0: 21])
    print("RSI", new_dataset['RSI'].iloc[-1])
    return new_dataset

def set_BB(new_dataset, period):
    new_dataset['20MA'] = new_dataset['close'].transform(lambda x: x.rolling(window=period).mean())
    new_dataset['SD'] = new_dataset['close'].transform(lambda x: x.rolling(window=period).std())
    new_dataset['upperband'] = new_dataset['20MA'] + 2*new_dataset['SD']
    new_dataset['lowerband'] = new_dataset['20MA'] - 2*new_dataset['SD']
    print("Upperband", new_dataset['upperband'].iloc[-1])
    print("Lowerband", new_dataset['lowerband'].iloc[-1])
    return new_dataset

def feature_engineering(dataset):
    # RSI (short for Relative Strength Index)
    # The standard is to use 14 periods to calculate RSI
    # (https://www.investopedia.com/terms/r/rsi.asp)
    period = 14
    new_dataset = set_RSI(dataset, period)

    ## BB (short for Bollinger Bands)
    # (https://www.investopedia.com/terms/b/bollingerbands.asp)
    # The standard is to use a period of 20
    per = 21
    new_dataset = set_BB(new_dataset, per)
    new_dataset = new_dataset[["unix", "RSI", "lowerband", "upperband","close"]]
    return new_dataset

def download_data(pairs, freq):
    ok_status_code = 200
    base_url = "https://www.cryptodatadownload.com/cdd"
    time_dict = {"hour": "1h", "min": "minute", "day": "d"}
    extension = ".csv"
    frequency = time_dict[freq]
    values = None
    threshold_ts = 60 * 60 * 6
    # hours from the last run (in seconds) which is already considered to have an out-of-date dataset
    for pair in pairs:
        filename = pair + extension
        values = clean_dataset(filename)
        if show_graph:
            print("RSI")
            RSI_plot(values)
            print("BB")
            BB_plot(values)

def BB_plot(values):
    x_axis = [x for x in range(len(values))]
    y_axis_one = values["lowerband"] 
    y_axis_two = values["upperband"]
    y_axis_thr = values["close"]
    plt.plot(x_axis, y_axis_one, label="Lower")
    plt.plot(x_axis, y_axis_two, label="Upper")
    plt.plot(x_axis, y_axis_thr, label="Value")
    plt.legend()
    plt.show()

def RSI_plot(values):
    x_axis = [x for x in range(len(values))]
    y_axis_one = values["RSI"] 
    plt.plot(x_axis, y_axis_one, label="RSI")
    plt.legend()
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    # (XYZ)/USDT support only due to data availability
    parser.add_argument(
        "--pairs", action='append', nargs='+', type=str,
        help="Desired cryptocurrency pair (support against USDT only), see https://coinmarketcap.com/exchanges/binance"
    )
    parser.add_argument("--freq", default="min", type=str, help="Frequency (choose from min, hour, day)")
    args = parser.parse_args()
    if args.pairs != None:
        pairs = flatten(args.pairs)
        freq = args.freq
        download_data(pairs, freq)
