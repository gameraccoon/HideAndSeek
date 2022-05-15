import pandas as pd
import plotly.express as px
import sys

# a value that we need our time to be multiplied on to get values in milleseconds
time_to_ms = 1000
# what precision we want to the values on the graph (the more the precision the noisier the graph)
rounding = 0.1
# how many decimal signs we want to show for time in the logs
digits_in_logs = 2
# extra frames at the beginning that we want to cut
extra_start_frames_to_cut = 2

df_base = pd.read_csv('./bin/frame_times_base.csv', names=["value"])
df = pd.read_csv('./bin/frame_times.csv', names=["value"])

# find the shortest data frame
count = df.count().value
count_base = df_base.count().value
min_count = min(count, count_base) - extra_start_frames_to_cut
begin_frame = extra_start_frames_to_cut
end_frame = extra_start_frames_to_cut + min_count

# clamp the data to contain records from the same frames
df = df.loc[begin_frame:end_frame]
df_base = df_base.loc[begin_frame:end_frame]

# convert to milliseconds rounded by some value
def round_ms(x):
    return round(x / time_to_ms / rounding) * rounding

# calculate frequencies
buckets = df.apply(lambda x: round_ms(x)).groupby("value").size()
buckets_base = df_base.apply(lambda x: round_ms(x)).groupby("value").size()

# merge into one data frame
buckets_merged = pd.DataFrame({"new":buckets, "base":buckets_base})

# print some additional information to the logs
print(f"Frames from {begin_frame} to {end_frame - 1}")

mean = df.mean().value / time_to_ms
mean_base = df_base.mean().value / time_to_ms
print(f"Base mean: {mean_base:.{digits_in_logs}f} ms New mean: {mean:.{digits_in_logs}f} ms")

median = df.median().value / time_to_ms
median_base = df_base.median().value / time_to_ms
print(f"Base median: {median_base:.{digits_in_logs}f} ms New median: {median:.{digits_in_logs}f} ms")

min_t = df.min().value / time_to_ms
mim_base = df_base.min().value / time_to_ms
print(f"Base min: {mim_base:.{digits_in_logs}f} ms New min: {min_t:.{digits_in_logs}f} ms")

max_t = df.max().value / time_to_ms
max_base = df_base.max().value / time_to_ms
print(f"Base max: {max_base:.{digits_in_logs}f} ms New max: {max_t:.{digits_in_logs}f} ms")

# if any command line arguments are provided, use them as the graph title
custom_title = " ".join(sys.argv[1:]).strip()
title = custom_title if len(custom_title) > 0 else 'Frame duration frequency (ms)'

# display graphs
#buckets_merged[buckets_merged.index<4].plot()#.get_figure().savefig('plot.png', dpi=600)
fig = px.line(buckets_merged, y = ["new", "base"], title=title)
fig.show()
