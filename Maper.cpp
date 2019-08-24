#include <bits/stdc++.h>

using namespace std;

int main(int argc, char **argv)
{
    //如果输入的参数不对，提示如何使用
    if (argc != 3)
    {
        printf("usage: %s FileName <Reducer_IP>\n",argv[0]);
        return 0;
    }
    //使用Map
    map<int, string> mapline;
    int index;
    //读取指定的文件
    ifstream in(argv[1]);
    //如果文件不存在，显示错误 
    if (!in.is_open())
        cout << "Error" << endl;
    else
    {
        //否则每次读取一行，存入光标位置和行内容为键值对。
        string buf;
        while (!in.eof())
        {
            getline(in, buf);//以行读取
            index = in.tellg();//得到光标位置
            if (index >= 0) //当为结尾时会返回负值
                mapline.insert(pair<int, string>(index, buf));//插入map中。
        }
    }
    map<int, string>::iterator iter; //用来迭代
    map<string, int> mapword; //用来存储单词
    multiset<string> setword;//用来得到不去重的单词
    for (iter = mapline.begin(); iter != mapline.end(); iter++)
    {
        istringstream sstr(iter->second); //将一行转换为单词
        string word;
        while (sstr >> word)
        {
            setword.insert(word); //插入set中
            mapword.insert(pair<string, int>(word, 1)); //插入Map中
        }
    }
    //将最后的结果写到文件中
    string res = argv[1];
    res += "_res";
    ofstream out(res.c_str());
    map<string, int>::iterator ite;
    for (ite = mapword.begin(); ite != mapword.end(); ite++)
    {
        ite->second = setword.count(ite->first); //统计每个单词个数。
        out << ite->first << ' ' << ite->second << endl;
    }
    //调用server_send_server将结果发送给Reducer.
    string exedoc = "./server_send_server ";
    string other_server_ip = argv[2];
    string trans_name = res;
    string send_port = "10012";
    string cmd = exedoc + other_server_ip +' '+ trans_name + ' ' + send_port;
    cout << cmd << endl;
    system(cmd.c_str()); //执行命令

    return 0;
}
