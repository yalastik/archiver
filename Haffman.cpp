#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include "Compressor.h"


using namespace std;


class Haffman
{
public:

    class Node
    {
    public:
        Node() : code(""),left(nullptr), right(nullptr) {}
        int value{};
        string code;
        Node *left, *right;
        int index{};
    };
    struct NodeCmp
    {
        bool operator()(const Node* x, const Node* y) const
        {
            if (x->value == y->value)
                return x->index < y->index;
            return x->value > y->value;
        }
    };


    Node *treeify()
    {
        while (queue.size() > 1)
        {
            Node *min1 = queue.top();
            queue.pop();
            Node *min2 = queue.top();
            queue.pop();

            Node *sum = new Node();
            sum->value = min1->value + min2->value;
            sum->index = count++;

            sum->right = min1;
            sum->left = min2;
            queue.push(sum);
        }

        return queue.top();
    }
    void reverse_code(Node* curr_node)
    {
        if (curr_node->left || curr_node->right)
        {
            curr_node->left->code = curr_node->code + '1';
            curr_node->right->code = curr_node->code + '0';
            reverse_code(curr_node->right);
            reverse_code(curr_node->left);
        } else
            codes[curr_node->index] = curr_node->code;
    }
    void build()
    {
        // зупскает алгоритм (после того как были добавлены все элементы)
        codes = new string[queue.size()];
        //строим дерево из нодов
        Node *root = treeify();
        //идем от корня дерева и добавляем нолики и единички детям, пока н наткнемся на лист
        reverse_code(root);


    }
    void addChance (int chance)
    {
        // добовляет елемент в список (дерево, все зависит от реализации)
        Node *newnode = new Node();
        newnode->value = chance;
        newnode->index = count++;
        queue.push(newnode);
    }
    string get (int i)
    {
        // выдает битовый код i символа
        return codes[i];
    }



    priority_queue<Node*,std::vector<Node*>, NodeCmp> queue;
    string *codes;
    int count = 0;



};

class Decoding_Haffman : public Decompressor
{
public:
    vector<char> read_table(ifstream &fin, Haffman *compressor)
    {
        file_length = 0;
        char symb;
        int len;

        fin.read((char*)&len,sizeof(int));
        vector<char> symbs;
        for (int i = 0; i < len; ++i)
        {
            int f;
            fin.read(&symb, sizeof(char));
            fin.read((char*) &f, sizeof(int));
            compressor->addChance(f);
            file_length += f;
            symbs.push_back(symb);
        }
        return symbs;
    }

    void write_decoded(ifstream &fin, ofstream &fout, Haffman *compressor, vector<char> symbs)
    {
        char symb;

        //the root of the tree
        auto root = compressor->treeify();
        auto cur_node = root;
        unsigned long length = symbs.size();

        //узнаем до какого места читать - то есть реально байты
        //последнее число - сколько битов из последнего байта - часть кода, а где нули
        fin.seekg(-sizeof(int), ios::end);
        long read_till = fin.tellg();
        int last_bit_end;
        fin.read((char*)&last_bit_end, sizeof(int));

        fin.seekg(sizeof(int) + length * (sizeof(int) + sizeof(char)));

        fin.read(&symb, sizeof(char));

        read_till -= sizeof(int) + length * (sizeof(int) + sizeof(char));

        while (--read_till > 0)
        {


            for (int i = 0; i < 8; ++i)
            {

                //идем по деревуб начиная с листа
                //встертили лист - нашли код - нашли символ по индексу этого кода в векторе символов
                //встретили битик 0 - пошли в правого ребенка
                if (!(cur_node->left || cur_node->right))
                {

                    fout.write(&symbs[cur_node->index], sizeof(char));
                    cur_node = root;

                }

                if ((int) ((symb >> i) & 1) == 1)
                    cur_node = cur_node->left;
                else
                    cur_node = cur_node->right;

            }
            fin.read(&symb, sizeof(char));
        }

        //дообработали последний бвайт с last_bit_end реальными битами
        for (int i = 0; i < last_bit_end; ++i)
        {

            if (!(cur_node->left || cur_node->right))
            {

                fout.write(&symbs[cur_node->index], sizeof(char));
                cur_node = root;

            }

            if ((int) ((symb >> i) & 1) == 1)
                cur_node = cur_node->left;
            else
                cur_node = cur_node->right;

        }

        if (!(cur_node->left || cur_node->right))
            fout.write(&symbs[cur_node->index], sizeof(char));
    }

        void decode(const string &filename_in, const string &filename_out)
        override {

        Haffman *compressor = new Haffman();


        ifstream fin;
        ofstream fout;
        fin.open(filename_in, ios::binary);

        vector<char> symbs = read_table(fin, compressor);


        fout.open(filename_out, ios::binary);
        write_decoded(fin, fout, compressor, symbs);
        fin.close();
        fout.close();

        delete compressor;
    }

//    map<string, char > code_to_char;
    int file_length;
};

class Encoding_Haffman : public Compressor
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

    double entropy(vector<pair<char, int>> sorted_freq, fpos<mbstate_t> length)
    {

    }

    vector<pair<char, int>> count_freq()
    {
        map<char, int> freq;
        ifstream fin;
        fin.open(filename_in, ios::binary | ios::ate);

        char symb;
        fpos<mbstate_t> length = fin.tellg();
        fin.seekg(0, ios::beg);

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

        Haffman *compressor = new Haffman();

        //читаем из файла символы и сортируем по убыванию частот
        sorted_freq = count_freq();

        //добавляем в очередь ноды
        for (auto &i : sorted_freq) {
            compressor->addChance(i.second);
        }
        //строим дерево
        compressor->build();

        //заполняем карту на основе кодов из массива кодов объекта компрессор
        int k = 0;
        for(auto & it : sorted_freq)
        {
            char_to_code[it.first] =  compressor->get(k);
            ++k;
        }
        //записываем таблицу и коды
        write_encoded();

        delete compressor;
    }

    map<char, string> char_to_code;
    vector<pair<char, int>> sorted_freq;
    string filename_in, filename_out;
};