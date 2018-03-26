// КДЗ по дисциплине Алгоритмы и структуры данных, 2017-2018 уч.год
// Савва Яна Максимовна, группа БПИ-163
// Не сделан LZW, все остальные алгоритмы реализованы

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include "Haffman.cpp"
#include "ShannonFano.cpp"
#include "LZ77.cpp"
#include "Compressor.h"
#include "Checker.cpp"
#include <cmath>

using namespace std;
using namespace std::chrono;


long file_length(const string &file_name);
void entropy_freqs(const string &file_name, ofstream & output);


int main() {


    //zero duration period
    nanoseconds s{0};
    //changing duration param
    nanoseconds tp;
    nanoseconds tu;

    Encoding_Haffman *encoding_haffman = new Encoding_Haffman();
    Encoding_Fano *encoding_fano = new Encoding_Fano();
    Encoding_LZ77 *encoding_lz775 = new Encoding_LZ77(4,1);
    Encoding_LZ77 *encoding_lz7710 = new Encoding_LZ77(8,2);
    Encoding_LZ77 *encoding_lz7720 = new Encoding_LZ77(16,4);



    Decoding_Haffman *decoding_haff = new Decoding_Haffman();
    Decoding_Fano *decoding_fano = new Decoding_Fano();
    Decoding_LZ77 *decoding_lz775 = new Decoding_LZ77(4, 1);
    Decoding_LZ77 *decoding_lz7710 = new Decoding_LZ77(8,2);
    Decoding_LZ77 *decoding_lz7720 = new Decoding_LZ77(16,4);


    Compressor *compressors[5] = {encoding_haffman, encoding_fano, encoding_lz775, encoding_lz7710, encoding_lz7720};
    Decompressor *decompressors[5] = {decoding_haff, decoding_fano, decoding_lz775, decoding_lz7710, decoding_lz7720};
    string ext_en[5] = {"haff", "shan", "lz775", "lz7710", "lz7720"};
    string ext_de[5] = {"unhaff", "unshan", "unlz775", "unlz7710", "unlz7720"};


    //формирование таблицы
    ofstream output;
//    output.open("results.csv");
    output.open("/media/yana/LENOVO/samples-for-students/results1.csv");

    //столбцы для энтропии, времен и коэфа
    output << "H,";
    for (int m = 0; m < 5; ++m) {
        output << "k,tp,tu,";
    }
    output << "\n";
    for (int i = 1; i <= 36; ++i)
    {
        string file_input(to_string(i / 10) + to_string(i % 10));
        output << file_input << ",";
        //count entropy
        entropy_freqs(file_input, output);
        long origin_length = file_length(file_input);
        for (int j = 0; j < 5; ++j)
        {
            tp = duration_cast<nanoseconds>(s);
            tu = duration_cast<nanoseconds>(s);
            double koefs = 0;
            //создаем имя для инкодид и декодид
            string file_encode("/media/yana/LENOVO/samples-for-students/" + file_input + "." + ext_en[j]);
            string file_decode("/media/yana/LENOVO/samples-for-students/" + file_input + "." + ext_de[j]);
            for (int k = 0; k < 20; ++k)
            {

                time_point<steady_clock> start = steady_clock::now();
                //кодируем
                compressors[j]->encode(file_input, file_encode);
                time_point<steady_clock> end1 = steady_clock::now();
                //декодируем
                cout << "file " + file_input << " compressed" <<"\t";

                decompressors[j]->decode(file_encode, file_decode);
                time_point<steady_clock> end2 = steady_clock::now();
                nanoseconds elapsed = duration_cast<nanoseconds>(end1 - start);


                //замеряем время
                tp += elapsed;
                elapsed = duration_cast<nanoseconds>(end2 - end1);
                tu += elapsed;


            }
            //считаем коэф
            koefs = (float)file_length(file_encode) / origin_length;
//            tp /= 20; tu /= 20;
            output << koefs << "," << tp.count() << "," << tu.count() << ",";

        }
        output << "\n";
    }


    output.close();

    for (int l = 0; l < 5; ++l)
    {
        delete compressors[l];
        delete decompressors[l];
    }
    return 0;
}

//подсчет длины файла
long file_length(const string &file_name)
{
    ifstream fin;
    fin.open(file_name, ios::binary | ios::ate);
    long size;
    if (fin.is_open())
        size = fin.tellg();
    fin.close();

    return size;
}

//подсчет энтропии и только при первом заходе - подсчет частот символов для всех файлов
void entropy_freqs(const string &file_name, ofstream &output)
{
    int symbols[256];
    for (int i = 0; i < 256; ++i) {
        symbols[i] = 0;
    }

    ifstream fin;
    fin.open(file_name, ios::binary | ios::ate);
    long size;
    if (fin.is_open())
        size = fin.tellg();

    fin.seekg(0, ios::beg);

    vector<unsigned char> bytes;
    unsigned char ch = fin.get();
    while (fin.good())
    {
        symbols[ch] += 1;
        ch = fin.get();
    }
    double H = 0;
//    ofstream freq_out("/home/yana/CLionProjects/KDZ/cmake-build-debug/samples-for-students/freqs/" + file_name + "_freq.csv");
    for (int j = 0; j < 256; ++j) {
        double pi = (double)symbols[j] / size;
//        freq_out << pi << ",";
        if (pi != 0)
            H -= (double)symbols[j] / size * log2(pi);
    }
    output << H << ",";
}