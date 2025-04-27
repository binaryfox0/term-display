import sys
import os
import re
import platform
import numpy as np
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QTabWidget, QWidget,
    QVBoxLayout, QFileDialog, QTableWidget, QTableWidgetItem,
    QLabel, QMessageBox
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import (QAction, QBrush, QColor)
import pyqtgraph as pg
from pyqtgraph.exporters import ImageExporter

def parse_log(path):
    timestamps, fps_values = [], []
    with open(path, "r") as f:
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

def detect_os_theme():
    if platform.system() == "Windows":
        try:
            import ctypes
            return ctypes.windll.dwmapi.DwmGetColorizationColor() & 0xFFFFFF < 0x808080
        except Exception:
            return False
    elif platform.system() == "Darwin":
        from subprocess import Popen, PIPE
        p = Popen(["defaults", "read", "-g", "AppleInterfaceStyle"], stdout=PIPE, stderr=PIPE)
        return p.communicate()[0].strip() == b"Dark"
    else:
        return os.getenv("XDG_CURRENT_DESKTOP", "").lower() in ["gnome", "kde", "xfce"]

class FPSGraph():
    def __init__(self, timestamps, fps_values):
        self.timestamps = timestamps
        self.fps_values = fps_values
        self.setup_color()
        self.plot_widget = pg.PlotWidget()
        self.setup_style()
        self.add_hover()

    def setup_color(self):
        self.dark_mode = detect_os_theme()
        self.bg_color = "#1E1E1E" if self.dark_mode else "#FFFFFF"
        self.fg_color = "#FFFFFF" if self.dark_mode else "#000000"
        self.line_color = "#00FF00"

    def setup_style(self):
        self.plot_widget.setBackground(self.bg_color)
        self.plot_widget.getAxis("left").setTextPen(self.fg_color)
        self.plot_widget.getAxis("bottom").setTextPen(self.fg_color)

        self.plot_widget.setLabel("bottom", "Time (seconds)", color=self.fg_color)
        self.plot_widget.setLabel("left", "FPS", color=self.fg_color)
        self.plot_widget.setTitle(f"FPS Performance Graph (Average: {np.mean(self.fps_values):.2f} FPS)", color=self.fg_color)

        self.plot = self.plot_widget.plot(self.timestamps, self.fps_values, pen=pg.mkPen(self.line_color, width=1))

    def add_hover(self):
        self.vLine = pg.InfiniteLine(angle=90, movable=False, pen=pg.mkPen("#888", style=pg.QtCore.Qt.PenStyle.DashLine))
        self.hLine = pg.InfiniteLine(angle=0, movable=False, pen=pg.mkPen((150, 150, 150, 80), style=pg.QtCore.Qt.PenStyle.DashLine))
        self.plot_widget.addItem(self.vLine, ignoreBounds=True)
        self.plot_widget.addItem(self.hLine, ignoreBounds=True)

        self.label = pg.TextItem("", anchor=(0, 1), color="#FF0", fill=QBrush(QColor(self.bg_color)))
        self.plot_widget.addItem(self.label)

        self.plot_widget.scene().sigMouseMoved.connect(self.mouse_moved)

    def mouse_moved(self, pos):
        vb = self.plot_widget.getViewBox()
        if vb.sceneBoundingRect().contains(pos):
            mousePoint = vb.mapSceneToView(pos)
            x, y = mousePoint.x(), mousePoint.y()
            idx = np.abs(self.timestamps - x).argmin()
            if 0 <= idx < len(self.timestamps):
                closest_x = self.timestamps[idx]
                closest_y = self.fps_values[idx]
                self.vLine.setPos(closest_x)
                self.hLine.setPos(closest_y)
                self.label.setText(f"{closest_y:.2f} FPS", color="#FF0")
                self.label.setPos(closest_x, closest_y)

class FPSViewer(QWidget):
    def __init__(self, timestamps, fps_values, log_file):
        super().__init__()
        self.timestamps = timestamps
        self.fps_values = fps_values
        self.log_file = log_file
        self.graph = FPSGraph(timestamps, fps_values)

    def get_tabs(self):
        return [
            ("FPS &Graph", self.create_graph_tab()),
            ("&Summary", self.create_summary_tab())
        ]

    def create_graph_tab(self):
        container = QWidget()
        layout = QVBoxLayout(container)
        layout.addWidget(self.graph.plot_widget)
        return container

    def create_summary_tab(self):
        container = QWidget()
        layout = QVBoxLayout(container)
        table = QTableWidget(6, 2)
        table.setHorizontalHeaderLabels(["Metric", "Value"])
        table.verticalHeader().setVisible(False)
        table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)

        duration = self.timestamps[-1] - self.timestamps[0] if len(self.timestamps) > 1 else 0
        data = [
            ("Min FPS", f"{np.min(self.fps_values):.2f}"),
            ("Max FPS", f"{np.max(self.fps_values):.2f}"),
            ("Avg FPS", f"{np.mean(self.fps_values):.2f}"),
            ("Duration", f"{duration:.2f} s"),
            ("Data Points", str(len(self.fps_values))),
            ("Log File", os.path.basename(self.log_file))
        ]

        for row, (key, value) in enumerate(data):
            table.setItem(row, 0, QTableWidgetItem(key))
            table.setItem(row, 1, QTableWidgetItem(value))

        layout.addWidget(table)
        return container

    
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("FPS Viewer")
        self.setGeometry(100, 100, 800, 500)

        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)

        # Initial tab with message
        self.placeholder_tab = QWidget()
        placeholder_layout = QVBoxLayout()
        self.placeholder_tab.setLayout(placeholder_layout)

        label = QLabel("Please go to File → Load to load program log")
        label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        label.setStyleSheet("font-size: 16pt;")
        placeholder_layout.addWidget(label)

        self.tabs.addTab(self.placeholder_tab, "Welcome")

        # Setup menu bar
        self.setup_menu()

    def setup_menu(self):
        menubar = self.menuBar()
        file_menu = menubar.addMenu("&File")

        load_action = QAction("L&oad", self)
        load_action.setShortcut("Ctrl+O")
        load_action.triggered.connect(self.load_log_file)

        export_action = QAction("&Export", self)
        export_action.setShortcut("Ctrl+E")
        export_action.triggered.connect(self.export_image)

        exit_action = QAction("E&xit", self)
        exit_action.setShortcut("Ctrl+Q")
        exit_action.triggered.connect(QApplication.quit)

        file_menu.addAction(load_action)
        file_menu.addAction(export_action)
        file_menu.addSeparator()
        file_menu.addAction(exit_action)

    def load_log_file(self):
        dialog = QFileDialog(self)
        dialog.setWindowTitle("Select FPS Log File")
        dialog.setNameFilter("Text files (*.txt);;All files (*)")

        if dialog.exec():
            log_file = dialog.selectedFiles()[0]
            timestamps, fps_values = parse_log(log_file)

            # Clear the existing tabs and load new ones
            self.tabs.clear()

            self.viewer = FPSViewer(timestamps, fps_values, log_file)
            for title, widget in self.viewer.get_tabs():
                self.tabs.addTab(widget, title)

            self.setWindowTitle(os.path.relpath(log_file))

    def export_image(self):
        if not hasattr(self, "viewer"):
            QMessageBox.warning(self, "No graph", "Please load a log file before exporting.")
            return
            
        file_dialog = QFileDialog(self)
        file_dialog.setAcceptMode(QFileDialog.AcceptMode.AcceptSave)
        file_dialog.setNameFilter("PNG Image (*.png);;SVG Image (*.svg)")
        file_dialog.setDefaultSuffix("png")
        file_dialog.setWindowTitle("Export Graph as Image")

        if file_dialog.exec():
            output_file = file_dialog.selectedFiles()[0]
            exporter = ImageExporter(self.viewer.graph.plot_widget.plotItem)
            exporter.export(output_file)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
