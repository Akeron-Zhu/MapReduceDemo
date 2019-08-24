#include <bits/stdc++.h>

using namespace std;

int main(int argc, char **argv)
{
    //参数个数不对则进行提示
    if (argc < 2)
    {
        printf("usage: ./Reducer FileName\n");
        return 0;
    }
    //进行Reduce操作
    map<string, int> wordReduce; 
    map<string, int>::iterator ite;
    string word;
    int count;
    //打开指定的文件
    ifstream in(argv[1]);
    if (in.is_open())
    {   //如果在map中还已有的，则加上原有数值。
        in >> word >> count;
        while (!in.eof())
        {
            // cout<<word<<' '<<count<<endl;
            ite = wordReduce.find(word);
            if (ite != wordReduce.end())
            {
                ite->second += count;
            }
            else
            { //没有的则插入。
                wordReduce.insert(pair<string, int>(word, count));
            }
            in >> word >> count;
        }
    }
    else
        cout << "Error" << endl;
    //输出结果
    ofstream out("ans");
    for (ite = wordReduce.begin(); ite != wordReduce.end(); ite++)
    {

        out << ite->first << ' ' << ite->second << endl;
    }
    remove("temp_ans");
    //将结果回传给客户端
    string exedoc = "./server_send_server ";
    string other_server_ip = "192.168.0.101 ";
    string trans_name = "ans";
    string send_port = "9999";
    string cmd = exedoc + other_server_ip + trans_name + ' ' + send_port;
    system(cmd.c_str()); //执行命令
    return 0;
}
