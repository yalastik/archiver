#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include "Compressor.h"

using namespace std;


class ShannonFano
{
public:

    void partition(int left, int right)
    {
        int sum = 0;

        for (int i = left; i <=right; ++i)
        {
            sum += seq[i];
        }
        float mean = float(sum) / 2;

        int right_part = 0;

        int lim = right;
        for (int i = right; i >= left; --i)
        {
            if (right_part + seq[i] <= sum)
            {
                codes[i] +='1';
                right_part += seq[i];
                lim = i;
                sum -= seq[i];
            }
            else
                codes[i] += '0';
        }

        if (lim - 1 > left) partition(left,lim-1);
        if (lim < right) partition(lim,right);


    }
    void build()
    {
        codes = new string[seq.size()];
        for (int i = 0; i < seq.size(); ++i)
            codes[i] = "";
        partition(0,seq.size()-1);

        // зупскает алгоритм (после того как были добавлены все элементы)

    }
    void addChance (int chance)
    {
        // добовляет елемент в список (дерево, все зависит от реализации)
        seq.push_back(chance);
    }
    string get (int i)
    {
        // выдает битовый код i символа
        return codes[i];
    }
    std::vector<int> seq;
    string *codes;

};

class Decoding_Fano : public Decompressor
{
public:
    vector<char> read_table(ifstream &fin, ShannonFano *compressor)
    {
//        file_length = 0;
        char symb = 0;
        int len;
        fin.seekg(0, ios::beg);

        fin.read((char*)&len, sizeof(int));
        vector<char> symbs;
        for (int i = 0; i < len; ++i)
        {
            int f;
            fin.read(&symb, sizeof(char));
            fin.read((char*) &f, sizeof(int));
            compressor->addChance(f);
//            file_length += f;
            symbs.push_back(symb);
        }
        return symbs;
    }

    void write_decoded(ifstream &fin, ofstream &fout, ShannonFano *compressor)
    {
        char symb = 0;
        string collected = "";
        fin.seekg(-sizeof(int), ios::end);
        long read_till = fin.tellg();
        int last_bit_end;
        fin.read((char*)&last_bit_end, sizeof(int));

        vector<char> symbs = read_table(fin, compressor);
        compressor->build();
        //карта код к чару заполняется после построения дерева и нахождения всех кодов
        //по симвалу из отсортированного вектора символов (по убыванию частот)
        map<string, char> code_to_char;

        int k = 0;
        for(char & symb : symbs)
        {
            code_to_char[compressor->get(k)] = symb;
            ++k;
        }

        unsigned long length = code_to_char.size();

        fin.seekg(sizeof(int) + length * (sizeof(int) + sizeof(char)), ios::beg);


        read_till -= sizeof(int) + length * (sizeof(int) + sizeof(char));
        fin.read(&symb, sizeof(char));

        while (--read_till > 0)
        {

            for (int i = 0; i < 8; ++i)
            {
                collected += ((int)((symb >> i) & 1) == 1)? "1" : "0";
                auto it = code_to_char.find(collected);
                //каждый раз при добавлении очередного прочитанного бита проверяем есть ли такой код в мэпе
                if (it != code_to_char.end())
                {
                    fout.write(&it->second, sizeof(char));
                    collected = "";
                }

            }
            fin.read(&symb, sizeof(char));
        }
        for (int i = 0; i < last_bit_end; ++i)
        {
            collected += ((int)((symb >> i) & 1) == 1)? "1" : "0";
            auto it = code_to_char.find(collected);
            if (it != code_to_char.end())
            {
                fout.write((char*)&it->second, 1);
                collected = "";
//                if (--file_length == 0)
//                    return;
            }

        }

    }

    void decode(const string &filename_in, const string &filename_out)
    override {

        ShannonFano *compressor = new ShannonFano();


        ifstream fin;
        ofstream fout;
        fin.open(filename_in, ios::binary);

//        vector<char> symbs = read_table(fin, compressor);
//        compressor->build();

//        int k = 0;
//        for(char & symb : symbs)
//        {
//            code_to_char[compressor->get(k)] = symb;
//            ++k;
//        }

        fout.open(filename_out, ios::binary);
        write_decoded(fin, fout, compressor);
        fin.close();
        fout.close();

        delete compressor;
    }

//    map<string, char > code_to_char;
//    vector<char> symbs;
//    int file_length;
};

class Encoding_Fano : public  Compressor
{
public:

    static bool freq_comparator(const pair<char,int>& first, const pair<char, int>& second)
    {
        return first.second > second.second;
    }

    void for_freq_diagram(vector<pair<char, int>> sorted_freq, fpos<mbstate_t> length)
    {
        ofstream fout;
        fout.open("/home/yana/CLionProjects/KDZ/cmake-build-debug/samples-for-students/freqs/"+filename_in + "_freq.csv", ios::trunc);

        for(auto it = sorted_freq.begin(); it != sorted_freq.end(); ++it)
        {
            fout << (unsigned int)it->first << ";";
        }
        fout << "\n";
        for(auto it = sorted_freq.begin(); it != sorted_freq.end(); ++it)
        {
            fout << (double) it->second / length << ";";
        }
        fout.close();
    }

    vector<pair<char, int>> count_freq()
    {
        map<char, int> freq;
        ifstream fin;
        fin.open(filename_in, ios::binary);

        char symb;
        fpos<mbstate_t> length = fin.tellg();
        fin.read(&symb, sizeof(char));
        while (!fin.eof())
        {
            if (freq.find(symb) == freq.end())
                freq[symb] = 1;
            else freq[symb] += 1;
            fin.read(&symb, sizeof(char));
        }

        fin.close();

        vector<pair<char, int>> sorted_freq(freq.begin(), freq.end());

//        for_freq_diagram(sorted_freq,length);


        sort(sorted_freq.begin(), sorted_freq.end(), freq_comparator);
        return sorted_freq;
    }

    void write_encoded()
    {
        ifstream fin;
        ofstream fout;
        fin.open(filename_in, ios::binary);
        fout.open(filename_out, ios::binary );

        //alphabet's symbols
        unsigned long amount = sorted_freq.size();
        //symbols in the file
//        fpos<mbstate_t> length = fin.tellg();

        fout.write((char*)(&amount), sizeof(int));
        for (auto it = sorted_freq.begin(); it != sorted_freq.end(); ++it)
        {
            fout.write(&it->first, sizeof(char));
            fout.write((char*)(&it->second), sizeof(int));
        }

        char symb;
        char added_bits = 0;
        int bits_length = 0;
        fin.read(&symb, sizeof(char));
        while (!fin.eof())
        {
            string code_i = char_to_code[symb];
            for (int j = 0; j < code_i.length(); ++j)
            {
                if(code_i[j] == '1')
                    added_bits = added_bits | (1 << bits_length);

                //увеличиваем счетчик записанных битиков в added_bits
                bits_length++;
                if (bits_length == 8)
                {
                    fout.write(&added_bits, sizeof(char));
                    added_bits = 0;
                    bits_length = 0;
                }
            }
            fin.read(&symb, sizeof(char));
        }
        fout.write(&added_bits, sizeof(char));
        fout.write((char*)&bits_length, sizeof(int));
        fin.close();
        fout.close();
    }


    void encode(const string &filename_in, const string &filename_out)
    override {
        this->filename_in = filename_in;
        this->filename_out = filename_out;

        ShannonFano *compressor = new ShannonFano();

        sorted_freq = count_freq();

        for (auto &i : sorted_freq) {
            compressor->addChance(i.second);
        }
        compressor->build();

        int k = 0;
        for(auto & it : sorted_freq)
        {
            char_to_code[it.first] =  compressor->get(k);
            ++k;
        }
        write_encoded();

        delete compressor;
    }

    map<char, string> char_to_code;
    vector<pair<char, int>> sorted_freq;
    string filename_in, filename_out;
};

//class Encoding_Fano : public  Compressor
//{
//public:
//
//    static bool freq_comparator(const pair<char,int>& first, const pair<char, int>& second)
//    {
//        return first.second > second.second;
//    }
//
//    vector<pair<char, int>> count_freq()
//    {
//        map<char, int> freq;
//        ifstream fin;
//        ofstream fout;
//        fin.open(filename_in, ios::binary | ios::ate);
//
//        char symb;
//        fpos<mbstate_t> length = fin.tellg();
//        for (int i = 0; i < length; ++i)
//        {
//            fin.seekg(i, ios::beg);
//            fin.read(&symb, 1);
//            if (freq.find(symb) == freq.end())
//                freq[symb] = 1;
//            else freq[symb] += 1;
//        }
//        fin.close();
//        auto it = freq.begin();
//
//        vector<pair<char, int>> sorted_freq(freq.begin(), freq.end());
//        sort(sorted_freq.begin(), sorted_freq.end(), freq_comparator);
//        return sorted_freq;
//    }
//
//    void write_encoded()
//    {
//        ifstream fin;
//        ofstream fout;
//        fin.open(filename_in, ios::binary | ios::ate);
//        fout.open(filename_out, ios::trunc | ios::binary | ios::ate);
//
//        //alphabet's symbols
//        unsigned long amount = sorted_freq.size();
//        //symbols in the file
//        fpos<mbstate_t> length = fin.tellg();
//
//        fout.write((char*)(&amount), 4);
//        for (auto it = sorted_freq.begin(); it != sorted_freq.end(); ++it)
//        {
//            fout.write(&it->first, 1);
//            fout.write((char*)(&it->second),4);
//        }
//
//        char symb;
//        char added_bits = 0;
//        int bits_length = 0;
//        for(int i = 0; i < length; ++i)
//        {
//            fin.seekg(i, ios::beg);
//            fin.read(&symb, 1);
//            string code_i = char_to_code[symb];
//            for (int j = 0; j < code_i.length(); ++j)
//            {
//                //смотрим на j-ый битик в текущем коде - переводим в инт
//                int bit_j = code_i[j] - '0';
//                //если это единичка - на позиции bits_length (сколько уже битиков записали в added_bits)
//                //записываем единичку
//                //иначе - при последующих итерациях встретившаяся единица встанет на нужное место (т.к. инкрементим bits_length)
//                if(bit_j)
//                    added_bits = added_bits | 1 << bits_length;
//                //увеличиваем счетчик записанных битиков в added_bits
//                bits_length++;
//                if (bits_length == 8)
//                {
//                    fout.write(&added_bits, 1);
//                    added_bits = 0;
//                    bits_length = 0;
//                }
//            }
//        }
//        if (added_bits != 0)
//            fout.write(&added_bits, 1);
//        fin.close();
//        fout.close();
//    }
//
//
//    void encode(const string &filename_in, const string &filename_out)
//    override {
//        this->filename_in = filename_in;
//        this->filename_out = filename_out;
//
//        ShannonFano *compressor = new ShannonFano();
//
//        sorted_freq = count_freq();
//        for (auto &i : sorted_freq) {
//            compressor->addChance(i.second);
//        }
//        compressor->build();
//
//        int k = 0;
//        for(auto & it : sorted_freq)
//        {
//            char_to_code[it.first] =  compressor->get(k);
//            ++k;
//        }
//        write_encoded();
//
//        delete compressor;
//    }
//
//    map<char, string> char_to_code;
//    vector<pair<char, int>> sorted_freq;
//    string filename_in, filename_out;
//};