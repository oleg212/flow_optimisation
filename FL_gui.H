#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_File_Chooser.H>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <regex>

#include "flow.H"
#include "flow_solver.H"
flow_solver solver;
class TrafficFlowGUI : public Fl_Window {
public:
    Fl_Input* input_data;
    Fl_Input* param_h;
    Fl_Input* param_alpha;
    Fl_Text_Display* output_area;
    Fl_Text_Display* log_area;
    Fl_Button* btn_group1;
    Fl_Button* btn_group2;
    Fl_Button* btn_open;
    Fl_Button* btn_crit;
    Fl_Button* btn_crit2;
    Fl_Button* btn_magic;

    TrafficFlowGUI(int w, int h, const char* title) : Fl_Window(w, h, title) {
        input_data = new Fl_Input(110, 10, 300, 200, "Исходный\nпоток");
        param_h = new Fl_Input(580, 10, 100, 30, "Параметр h:");
        btn_open = new Fl_Button(580, 50, 110, 30, "Открыть файл");
        btn_group1 = new Fl_Button(460, 90, 110, 30, "Группировка 1");
        btn_group2 = new Fl_Button(580, 90, 110, 30, "Группировка 2");
        param_alpha = new Fl_Input(580, 130, 100, 30, "Параметр alpha:");
        btn_crit = new Fl_Button(460, 170, 230, 30, "Проверка независимости");
        btn_magic = new Fl_Button(460, 210, 230, 50, "Автоматическая группировка");
        output_area = new Fl_Text_Display(110, 290, 300, 200, "Сгруппированный \nпоток");
        log_area = new Fl_Text_Display(450, 290, 300, 200, "Лог");

        btn_group1->callback(group1_cb, this);
        btn_group2->callback(group2_cb, this);
        btn_open->callback(open_file_cb, this);
        btn_crit->callback(crit_cb, this);
        // btn_crit2->callback(crit2_cb, this);
        // btn_magic->callback(magic_cb, this);

        Fl_Text_Buffer* output_buffer = new Fl_Text_Buffer();
        output_area->buffer(output_buffer);
        Fl_Text_Buffer* log_buffer = new Fl_Text_Buffer();
        log_area->buffer(log_buffer);
    }

    static void group1_cb(Fl_Widget* w, void* data) {
        TrafficFlowGUI* gui = static_cast<TrafficFlowGUI*>(data);
        std::string data_str = gui->input_data->value();
        if (data_str.empty()) {
            gui->log("Ошибка: введите данные потока!");
            return;
        }
        flow potok = parse_input(data_str);
        double h;
        try {
            h = std::stod(gui->param_h->value());
        } catch (...) {
            gui->log("Ошибка: некорректное значение h!");
            return;
        }

        flow potok2 = solver.batches_compute_1(potok, h);
        std::ostringstream oss;
        oss << potok2;
        gui->output_area->buffer()->text(oss.str().c_str());
    }

    static void open_file_cb(Fl_Widget* w, void* data) {
            TrafficFlowGUI* gui = static_cast<TrafficFlowGUI*>(data);
            Fl_File_Chooser chooser(".", "*.txt", Fl_File_Chooser::SINGLE, "Открыть файл");
            chooser.show();
            while (chooser.visible()) {
                Fl::wait();
            }
            if (chooser.value()) { // Check if a file was selected
                std::ifstream file(chooser.value());
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    gui->input_data->value(buffer.str().c_str());
                    file.close();
                } else {
                    gui->log("Ошибка: не удалось открыть файл");
                }
            }
        }

    static void group2_cb(Fl_Widget* w, void* data) {
        TrafficFlowGUI* gui = static_cast<TrafficFlowGUI*>(data);
        std::string data_str = gui->input_data->value();
        if (data_str.empty()) {
            gui->log("Ошибка: введите данные потока!");
            return;
        }
        flow potok = parse_input(data_str);
        double h;
        try {
            h = std::stod(gui->param_h->value());
        } catch (...) {
            gui->log("Ошибка: некорректное значение С!");
            return;
        }

        flow potok2 = solver.batches_compute_4(potok, h);
        std::ostringstream oss;
        oss << potok2;
        gui->output_area->buffer()->text(oss.str().c_str());
    }
    
    private:

    void log(std::string msg)
    {
        log_area->buffer()->append(msg.c_str());
        log_area->buffer()->append("\n");
    }

    static flow parse_input(const std::string& data_str) {
        std::vector<cross> crosses;
        std::string cleaned_data = data_str;

        // Удаляем префикс "flow level X" если есть
        std::regex level_regex("flow level \\d+\\s*");
        cleaned_data = std::regex_replace(cleaned_data, level_regex, "");

        // Проверяем, содержит ли строка формат "(time; n = count)"
        if (cleaned_data.find('(') != std::string::npos) {
            // Формат 2 или 3: парсим пары (time; n = count)
            std::regex pair_regex("\\(\\s*([0-9.]+)\\s*;\\s*n\\s*=\\s*(\\d+)\\s*\\)");
            std::sregex_iterator it(cleaned_data.begin(), cleaned_data.end(), pair_regex);
            std::sregex_iterator end;

            for (; it != end; ++it) {
                double time = std::stod(it->str(1));
                count n = std::stoi(it->str(2));
                crosses.emplace_back(time, n);
            }
        } else {
            // Формат 1: простая последовательность чисел
            std::stringstream ss(cleaned_data);
            std::string token;
            while (std::getline(ss, token, ',')) {
                token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
                if (!token.empty()) {
                    double time = std::stod(token);
                    crosses.emplace_back(time, 1);
                }
            }
        }

        return flow(crosses);
    }

    static void crit_cb(Fl_Widget* w, void* data) {
        TrafficFlowGUI* gui = static_cast<TrafficFlowGUI*>(data);
        std::string data_in_str = gui->input_data->value();
        if (data_in_str.empty()) {
            gui->log("Ошибка: введите данные входного потока!");
        }
        flow potok_in = parse_input(data_in_str);

        std::string data_out_str = gui->input_data->value();
        if (data_out_str.empty()) {
            gui->log("Ошибка: рассчитайте выходной поток!");
        }
        flow potok_out = parse_input(data_out_str);

        double alpha;
        try {
            alpha = std::stod(gui->param_h->value());
        } catch (...) {
            gui->log("Ошибка: некорректное значение alpha!");
            return;
        }
        alpha = 0.05;

        if (potok_in.crosses.size())
        {
            bool res_1 = solver.criterion_1(potok_in, alpha);
            gui->log("Входной поток, критерий 1: " + res_1 ? "True" : "False");
            bool res_2 = solver.criterion_2(potok_in, alpha);
            gui->log("Входной поток, критерий 2: " + res_2 ? "True" : "False");
        }
        if (potok_out.crosses.size())
        {
            bool res_1 = solver.criterion_1(potok_out, alpha);
            gui->log("Выходной поток, критерий 1: " + res_1 ? "True" : "False");
            bool res_2 = solver.criterion_2(potok_out, alpha);
            gui->log("Выходной поток, критерий 2: "+ res_2 ? "True" : "False");
        }
        
    }

};

