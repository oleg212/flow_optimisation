#include "FL_gui.H"


int main() {
    TrafficFlowGUI* gui = new TrafficFlowGUI(900, 600, "GUI для анализа потока");
    gui->show();
    return Fl::run();
}