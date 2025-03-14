import re
import argparse
import numpy as np
import matplotlib.pyplot as plt
import os
import platform

# Display style only
def detect_os_theme():
 if platform.system() == "Windows":
  import ctypes
  try:
   is_dark_mode = ctypes.windll.dwmapi.DwmGetColorizationColor() & 0xFFFFFF < 0x808080
  except AttributeError:
   is_dark_mode = False
 elif platform.system() == "Darwin":
  from subprocess import Popen, PIPE
  p = Popen(["defaults", "read", "-g", "AppleInterfaceStyle"], stdout=PIPE, stderr=PIPE)
  is_dark_mode = p.communicate()[0].strip() == b"Dark"
 else:
  is_dark_mode = os.getenv("XDG_CURRENT_DESKTOP", "").lower() in ["gnome", "kde", "xfce"]

 return is_dark_mode


def parse_log(path):
 timestamps, fps_values = [], []
 with open(path, "r") as f:
  # Log format: [%d:%02d.%06d]: FPS: %d.%06d
  pattern = re.compile(r"\[(\d+):(\d+)\.(\d+)\]: FPS: (\d+)\.(\d+)")
  for line in f:
   match = pattern.match(line)
   if match:
    minutes, seconds, microseconds, fps_int, fps_frac = map(int, match.groups())
    time_in_seconds = minutes * 60 + seconds + microseconds / 1e6
    fps = fps_int + fps_frac / 1e6
    timestamps.append(time_in_seconds)
    fps_values.append(fps)
 
 return np.array(timestamps), np.array(fps_values)

# Event handling
def update_annot(event):
 """ Update annotation when cursor is near a line """
 if event.inaxes == ax:
  xdata, ydata = line.get_xdata(), line.get_ydata()
  idx = np.argmin(np.abs(xdata - event.xdata))  # Find nearest x-point
  x_nearest, y_nearest = xdata[idx], ydata[idx]
  
  annot.xy = (x_nearest, y_nearest)
  annot.set_text(f"{y_nearest:.2f} FPS")
  annot.set_visible(True)
  fig.canvas.draw_idle()

def hover(event):
 """ Hide annotation when cursor leaves the plot """
 if event.inaxes != ax:
  annot.set_visible(False)
  fig.canvas.draw_idle()

def setup_plot(bg_color, text_color):
 ax.set_facecolor(bg_color)
 fig.patch.set_facecolor(bg_color)
 ax.spines["top"].set_visible(False)
 ax.spines["right"].set_visible(False)
 ax.spines["left"].set_color(text_color)
 ax.spines["bottom"].set_color(text_color)
 ax.tick_params(axis="x", colors=text_color)
 ax.tick_params(axis="y", colors=text_color)
 ax.xaxis.label.set_color(text_color)
 ax.yaxis.label.set_color(text_color)
 ax.title.set_color(text_color)

if __name__ == '__main__':
 # Argument parsing
 parser = argparse.ArgumentParser(description="Visualizing FPS over time.")
 parser.add_argument("input_log", type=str, help="Path to the log file")
 parser.add_argument("--transparent", action="store_true", help="Enable transparent background")
 parser.add_argument("--output", "-o", nargs="?", metavar="FILE", help="Output line graph into picture")
 args = parser.parse_args()

 timestamps, fps_values = parse_log(args.input_log)
 average_fps = np.mean(fps_values)

 # Create the plot
 fig, ax = plt.subplots(figsize=(10, 5))

 # Detect OS theme (dark mode or light mode)

 dark_mode = detect_os_theme()
 color_table = [
  ["#FFFFFF", "#1E1E1E"],
  ["#00FF00", "#00FF00"],
  ["#DDDDDD", "#444444"],
  ["#000000", "#FFFFFF"]
 ]
 bg_color = color_table[0][dark_mode]
 line_color = color_table[1][dark_mode]
 grid_color = color_table[2][dark_mode]
 text_color = color_table[3][dark_mode]
 annot_outline_color = color_table[0][not dark_mode]

 setup_plot(bg_color, "None" if args.transparent else text_color)

 # Plot FPS line
 line, = ax.plot(timestamps, fps_values, color=line_color, linewidth=0.5)

 # Grid settings
 ax.grid(True, linestyle="--", linewidth=0.5, color=grid_color)

 # Labels and title
 ax.set_xlabel("Time (seconds)")
 ax.set_ylabel("FPS")
 ax.set_title(f"FPS Performance Graph (Average: {average_fps:.2f} FPS)")

 # Hover interaction
 annot = ax.annotate("", xy=(0, 0), xytext=(10, 10),
  textcoords="offset points", color=text_color,
  bbox=dict(fc=bg_color, edgecolor=annot_outline_color),
  arrowprops=dict(arrowstyle="->", color=annot_outline_color))
 annot.set_visible(False)

 fig.canvas.mpl_connect("motion_notify_event", update_annot)
 fig.canvas.mpl_connect("figure_leave_event", hover)

 # Save or show the graph
 if args.output:
  plt.savefig(args.output, transparent=args.transparent, dpi=300)
 else:
  fig.canvas.manager.set_window_title("FPS Visualizer")
  plt.show()
