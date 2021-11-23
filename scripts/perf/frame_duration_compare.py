import pandas as pd
import plotly.express as px
import sys

df_base = pd.read_csv('./bin/frame_times_base.csv', names=["value"])
df = pd.read_csv('./bin/frame_times.csv', names=["value"])

# extra frames at the beginning that we want to cut
extra_start_frames_to_cut = 2

# find the shortest data frame
count = df.count().value
count_base = df_base.count().value
min_count = min(count, count_base) - extra_start_frames_to_cut

# clamp the data to contain records from the same frames
df = df.loc[extra_start_frames_to_cut:min_count - 1]
df_base = df_base.loc[extra_start_frames_to_cut:min_count - 1]

# convert to milliseconds rounded by 0.1
def round_ms(x):
    return round(x / 100000) / 10.0

# calculate frequencies
buckets = df.apply(lambda x: round_ms(x)).groupby("value").size()
buckets_base = df_base.apply(lambda x: round_ms(x)).groupby("value").size()

# merge into one data frame
buckets_merged = pd.DataFrame({"new":buckets, "base":buckets_base})

# print some additional information
print("Frames from {} to {}".format(extra_start_frames_to_cut, extra_start_frames_to_cut + min_count - 1))

mean = df.mean().value / 1000000
mean_base = df_base.mean().value / 1000000
print("Base mean: {:.2f} ms New mean: {:.2f}ms".format(mean_base, mean))

median = df.median().value / 1000000
median_base = df_base.median().value / 1000000
print("Base median: {:.2f} ms New median: {:.2f}ms".format(median_base, median))

# if any command line arguments provided, use them as graph title
custom_title = " ".join(sys.argv[1:]).strip()
title = custom_title if len(custom_title) > 0 else 'Frame duration frequency (ms)'

# display graphs
#buckets_merged[buckets_merged.index<4].plot()#.get_figure().savefig('plot.png', dpi=600)
fig = px.line(buckets_merged, y = ["new", "base"], title=title)
fig.show()
