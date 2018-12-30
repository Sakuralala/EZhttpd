//一些东西的测试，很杂的
#include "test.h"
#include <bits/stdc++.h>
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <unistd.h>

using namespace std;


/*
//测试explicit，
class A
{
public:
    A () {}
    explicit A (const string &s) {}
    void func1(const A& a){}
};

int main()
{
   // A a("shit"); //ok
    //A a2 = string("shit"); //wrong
因为转换构造函数是explicit的,即不接受隐式的转换,只能显式调用
}
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;
int
main ()
{
    //学生个数
    int n = 36;
    //学生能力值
    vector<int> vi{7,   -15, 31,  49,  -44, 35,  44, -47, -23, 15,
                   -11, 10,  -21, 10,  -13, 0,   -20,     -36, 22,
                   -13, -39, -39, -31, -13, -27, -43,     -6,  40,
                   5,   -47, 35,  -8,  24,  -31, -24,     -1};
    //选取的学生个数和最大间隔
    int k = 3, d = 31;

    //存储最大值最小值
    vector<vector<long long>> Max (k, vector<long long> (n, 0));
    vector<vector<long long>> Min (k, vector<long long> (n, 0));

    for (int i = 1; i <= k; ++i)
    {
        for (int j = i - 1; j < (n - k + i); ++j)
        {
            if (i == 1)
            {
                Max[i - 1][j] = vi[j];
                Min[i - 1][j] = vi[j];
            }
            else
            {
                int beg = (j - d) > 0 ? (j - d) : 0;
                long long max_elem = *max_element (Max[i - 2].begin () + beg,
                                                   Max[i - 2].begin () + j);
                long long min_elem = *min_element (Min[i - 2].begin () + beg,
                                                   Min[i - 2].begin () + j);
                Max[i - 1][j] = vi[j] >= 0 ? max_elem * vi[j]
                                           : min_elem * vi[j];
                Min[i - 1][j] = vi[j] >= 0 ? min_elem * vi[j]
                                           : max_elem * vi[j];
            }
        }
    }
    cout << *max_element (Max[k - 1].begin (), Max[k - 1].end ()) << endl;
    getchar();
}
*/

/*
#include <algorithm>
#include <iostream>
#include <memory.h>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::swap;
using std::vector;
using std::sort;
int
cal_cnt (const vector<int> &vi)
{
    int ret = 0;
    for (int i = 0; i < vi.size (); ++i)
    {
        if (vi[i])
        {
            for (int j = i + 1; j < vi.size (); ++j)
                if (vi[j] && vi[i] < vi[j])
                    ret++;
        }
    }

    return ret;
}

void
help_func (const vector<int> &vi1, vector<int> &vi, int start, int &valid_cnt,
           const int k)
{
    if (start == vi.size () - 1)
    {
        vector<int> tmp (vi1);
        for (int i = 0, j = 0; i < tmp.size (); ++i)
            if (!tmp[i])
            {
                tmp[i] = vi[j++];
            }
        int cnt = cal_cnt (tmp);
        if (cnt == k)
        {
            valid_cnt++;
            // cout<<cnt1<<cnt2<<cnt3<<endl;
        }
        return;
    }
    for (int i = start; i < vi.size (); ++i)
    {
        if (i != start)
            swap (vi[i], vi[start]);
        help_func (vi1, vi, start + 1, valid_cnt, k);
        if (i != start)
            swap (vi[i], vi[start]);
    }
}

int
main ()
{
    int n = 100, k = 2405;
     //cin >> n >> k;

     vector<int> vi(n);
     char is_zero[n / sizeof(char) / 8 + 1];
     memset(is_zero, 0, n / 8 + 1);

     for (auto& elem : vi)
     {
         cin >> elem;
         // cout << elem << endl;
         if (elem)
             is_zero[elem / 8 / sizeof(char)] |= (1 << (elem % (8 *
sizeof(char))));
    }
    sort(vi.begin(),vi.end());
    for(auto elem:vi)
        cout << elem <<" ";

    cout << endl;
    int cnt1 = cal_cnt (vi);

    if (cnt1 > k)
    {
        cout << 0 << endl;
        return 0;
    }
    //把看不清的元素保存起来
    vector<int> zero;
    for (int i = 1; i <= n; ++i)
        if ((is_zero[i / sizeof (char)/8] & (1 << (i % (8*sizeof (char))))) ==
0)
        {
            zero.push_back (i);
            cout << i << endl;
        }

    int cur_cnt = 0;
    help_func (vi, zero, 0, cur_cnt, k);

    cout << cur_cnt << endl;
    getchar ();
    return 0;
}
*/

/*
void
dfs (const vector<int> &vi, int index, int sum, int mul, int &cnt,
     vector<int> &tmp, vector<vector<int>> &vvi)
{
    for (int i = index; i < vi.size (); ++i)
    {
        //袋中尚无元素
        if (sum == 0 && mul == 1)
        {
            //起始元素
            tmp.clear();
            tmp.push_back(vi[i]);
            dfs (vi, i + 1, sum+vi[i], mul*vi[i], cnt,tmp,vvi);

        }
        else
        {
            //满足条件
            if ((sum + vi[i]) > (mul * vi[i]))
            {
                cnt++;
                tmp.push_back(vi[i]);
                vvi.push_back(tmp);
                dfs (vi, i + 1, sum + vi[i], mul * vi[i], cnt,tmp,vvi);
                //要切到下一个元素了，所以先要删掉当前i指向的元素
                tmp.pop_back();
            }
            else
            {
                break;
            }
        }
        //重复元素的剪枝
        while ((i < vi.size ()-1) && vi[i + 1] == vi[i])
            i++;
    }
}
int
main ()
{
    // cin >> n;
    vector<int> vi{1, 1, 1,1,2,2,2,2};
    int cnt = 0;
    // O(NlogN)
    sort (vi.begin (), vi.end ());
    vector<int> tmp;
    vector<vector<int>> vvi;
    dfs (vi, 0, 0, 1, cnt,tmp,vvi);
    for(auto& elem:vvi)
    {
        for(auto& elem1:elem)
            cout << elem1 << "  ";
        cout << endl;
    }
    cout << cnt << endl;
    return 0;
}
*/
/**
#ifdef debug
#define tracking 24
#define n 4
#define m 24
#endif
**/

/**
int min(const int a, const int b)
{
    return a > b ? b : a;
}
int get_min(const int src, const int dest)
{
    //到了
    if (dest == src)
        return 0;
    //调过头了
    if (dest < src)
        return -1;
    int cnt = 100000;
    for (int i = 2; i <= src / 2; ++i)
    {
        if (src + i <= dest)
        {
            //因子
            if (src / i * i == src)
            {
                int sub_cnt = get_min(src + i, dest);
                if (sub_cnt != -1)
                    cnt = min(1 + sub_cnt, cnt);
            }
        }
        else
            break;
    }
    //没有因子
    if (cnt == 100000)
        return -1;
    return cnt;
}

int main()
{
    int n = 4, m = 24;
    //cin >> n >> m;

    if (m == n)
        cout << 0 << endl;
    if (m - n == 1)
        cout << -1 << endl;
    cout << get_min(n, m) << endl;
    getchar();
}
**/
/*
int main()
{
    int n = 321, m = 54237;
    // cin >> n >> m;

    if (m == n)
        cout << 0 << endl;
    if (m - n == 1)
        cout << -1 << endl;
    vector<int> vi(m - n + 1, INT_MAX);
    vi[0] = 0;
    for (int i = 0; i < vi.size() - 2; ++i)
    {
        if (vi[i] == INT_MAX)
            continue;
        for (int j = 2; j * j <= (n + i); ++j)
        {
            if ((n + i) % j == 0)
            {
                if ((i + j) <= (m - n))
                    vi[i + j] = min(vi[i + j], 1 + vi[i]);
                else
                    break;
                if ((i + (n + i) / j) <= (m - n))
                    vi[i + (n + i) / j] = min(vi[i + (n + i) / j], vi[i] + 1);
            }
        }
    }
    if (vi[m - n] == INT_MAX)
        cout << -1 << endl;
    else
        cout << vi[m - n] << endl;
    getchar();
}

struct Comp
{
    bool operator()(const pair<long long, long long> &a,
                    const pair<long long, long long> &b) const
    {
        return a.second < b.second;
    }
};

int b_search(const vector<long long> &vi, const long long val)
{
    int l = 0, r = vi.size() - 1;
    while (l <= r)
    {
        int m = (l + r) / 2;
        if (vi[m] > val)
            r = m - 1;
        else if (vi[m] == val)
            return m;
        else
            l = m + 1;
    }

    return l;
}

int main()
{
    int n = 0, m = 0;
    cin >> n >> m;
    vector<long long> desk;
    vector<pair<long long, long long>> person_cnt;

    //桌子容量排序
    while (n--)
    {
        long long a = 0;
        cin >> a;
        desk.push_back(a);
    }
    sort(desk.begin(), desk.end());
    //金额从大到小
    while (m--)
    {
        long long person = 0, cnt = 0;
        cin >> person >> cnt;
        if (person > desk[desk.size() - 1])
            continue;
        person_cnt.push_back(make_pair(person, cnt));
    }
    sort(person_cnt.begin(), person_cnt.end(), Comp());
    long long total = 0;
    for (int i = person_cnt.size() - 1; i >= 0 && (!desk.empty()); --i)
    {
        if (person_cnt[i].first <= desk[desk.size() - 1])
        {
            int index = b_search(desk, person_cnt[i].first);
            total += person_cnt[i].second;
            desk.erase(desk.begin() + index);
        }
        person_cnt.pop_back();
    }

    cout << total << endl;
    return 0;
}
vector<pair<int, int>> min_path;
void func(vector<pair<int, int>> &vp, vector<vector<bool>> &visited,
          const vector<vector<int>> &vvi, const int i, const int j, int p)
{
    if ((i == 0) && (j == vvi[0].size() - 1))
    {
        if (min_path.size() == 0 || min_path.size() > vp.size())
            min_path = vp;
    }
    else
    {
        //下
        if (i < (vvi.size() - 1) && vvi[i + 1][j] == 1 &&
            visited[i + 1][j] == false)
        {
            visited[i + 1][j] = true;
            vp.push_back(make_pair(i + 1, j));
            func(vp, visited, vvi, i + 1, j, p);
            vp.pop_back();
            visited[i + 1][j] = false;
        }
        //上
        if (i > 0 && vvi[i - 1][j] == 1 && visited[i - 1][j] == false &&
            (p - 3) >= 0)
        {
            visited[i - 1][j] = true;
            vp.push_back(make_pair(i - 1, j));
            p -= 3;
            func(vp, visited, vvi, i - 1, j, p);
            p += 3;
            vp.pop_back();
            visited[i - 1][j] = false;
        }
        //左
        if (j > 0 && vvi[i][j - 1] == 1 && visited[i][j - 1] == false &&
            (p - 1) >= 0)
        {
            visited[i][j - 1] = true;
            vp.push_back(make_pair(i, j - 1));
            p -= 1;
            func(vp, visited, vvi, i, j - 1, p);
            p++;
            vp.pop_back();
            visited[i][j - 1] = false;
        }
        //右
        if (j < (vvi[0].size() - 1) && vvi[i][j + 1] == 1 &&
            visited[i][j + 1] == false && (p - 1) >= 0)
        {
            visited[i][j + 1] = true;
            vp.push_back(make_pair(i, j + 1));
            p--;
            func(vp, visited, vvi, i, j + 1, p);
            p++;
            vp.pop_back();
            visited[i][j + 1] = false;
        }
    }
}

int main()
{
    string str = "";
    cout << str.size() << endl;
    cout << '2' - '1' << endl;
    int n = 0, m = 0, p = 0;
    cin >> n >> m >> p;
    vector<vector<int>> vvi(n, vector<int>(m, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            cin >> vvi[i][j];
    vector<pair<int, int>> vp;
    vector<vector<bool>> visited(n, vector<bool>(m, 0));
    visited[0][0] = true;
    vp.push_back(make_pair(0, 0));
    func(vp, visited, vvi, 0, 0, p);
    if (min_path.size() > 0)
    {
        for (int i = 0; i < min_path.size(); ++i)
        {
            cout << "[" << min_path[i].first << "," << min_path[i].second <<
"]"; if (i < min_path.size() - 1) cout << ","; else cout << endl;
        }
    }
    else
        cout << "Can not escape!" << endl;
    return 0;
}

int main()
{
    string cur_str, max_str;
    cin >> cur_str;
    cout << cur_str;
    char c = '0';
    while ((c = getchar()) != '\n')
    {
        if (cur_str.size() == 0)
            cur_str += c;
        else
        {
            if (c - cur_str[cur_str.size() - 1] == 1)
                cur_str += c;
            else
            {
                if (cur_str.size() > max_str.size())
                    max_str = std::move(cur_str);
                cur_str = "";
            }
        }
    }
    cout << max_str;

    getchar();
    return 0;
}

void func(vector<int> &vi, int cur, int m, const int n)
{
    if (m == 0)
    {
        for (int i = 0; i < vi.size(); ++i)
            if (i != vi.size() - 1)
                cout << vi[i] << " ";
            else
                cout << vi[i] << endl;
    }
    else
    {
        for (int i = cur; i <= n; ++i)
        {
            if (m - i >= 0)
            {
                vi.push_back(i);
                func(vi, i + 1, m - i, n);
                vi.pop_back();
            }
            else
                break;
        }
    }
}
class A
{
  public:
    A() {}
    A(const string &s) {}
};

int &func1(const int a, const int b, int &c)
{
    c = a + b;
    return c;
}

int main()
{
    int cc = 0;
    auto dd = func1(1,2,cc)+1;
    cout << dd<<cc<< endl;
    A a("shit");
    A b = a;
    A c = A("shit");
    A d = string("shit");
    cout << 'q' - 'a' << endl;
    int n = 0, m = 0;
    cin >> n >> m;
    vector<int> vi;
    func(vi, 1, m, n);
    return 0;
}

class cmp
{
  public:
    bool operator()(const pair<pair<int, int>, int> &a, const pair<pair<int,
int>, int> &b) const
    {
        if (a.first.first < b.first.first)
            return true;
        if (a.first.first == b.first.first)
            return a.first.second < b.first.second;
        return false;
    }
};

int main()
{
    int arr[10001];
    memset(arr, -1, sizeof(arr));
    int d1, d2, pri, min = 0x7fffffff, max = -1;
    ;
    while (cin >> d1 >> d2 >> pri)
    {
        for (int i = d1; i <= d2; ++i)
            arr[i] = pri;
        if (min > d1)
            min = d1;
        if (max < d2)
            max = d2;
    }

    for (int i = min; i <= max; ++i)
    {
        if (arr[i] != -1)
        {
            int j = i + 1;
            while (j <= max)
            {
                if (arr[j] == arr[i])
                    j++;
                else
                    break;
            }
            cout << "[" << i << ", " << j - 1 << ", " << arr[i] << "]";
            if (j <= max)
                cout << ",";
            else
                cout << "shit";
            i = j - 1;
        }
    }

    getchar();
    return 0;
}

int main()
{
    map<pair<int, int>, int> mp;
    int a = 0, b = 0, p = 0;
    while (cin >> a >> b >> p)
    {
        if (!mp.empty())
        {
            auto ite = mp.lower_bound(make_pair(a, b));
            if (ite != mp.begin())
                --ite;
            while (ite != mp.end())
            {
                if (ite->first.first > b)
                    break;
                else if (b < ite->first.second)
                {
                    if (ite->first.first >= a)
                    {
                        (const_cast<pair<int, int> &>(ite->first)).first = b +
1;
                    }
                    else
                    {
                        mp.insert(
                            make_pair(make_pair(b + 1, ite->first.second),
ite->second)); (const_cast<pair<int, int> &>(ite->first)).second = a - 1;
                    }
                }
                else
                {
                    if (a <= ite->first.first)
                    {
                        ite = mp.erase(ite);
                        --ite;
                    }
                    else if (a <= ite->first.second)
                        (const_cast<pair<int, int> &>(ite->first)).second = a -
1;
                }
                ++ite;
            }
        }
        mp.insert(make_pair(make_pair(a, b), p));
        cout << "Current map:" << endl;
        for (auto ite = mp.begin(); ite != mp.end(); ++ite)
            cout << ite->first.first << " " << ite->first.second << " " <<
ite->second
                 << endl;
        cout << endl;
    }

    for (auto ite = mp.begin(); ite != mp.end(); ++ite)
    {
        auto ite1 = ite;
        ++ite1;
        while (ite1 != mp.end())
        {
            if (ite1->second == ite->second &&
                ite1->first.first == ite->first.second + 1)
                ++ite1;
            else
                break;
        }
        --ite1;
        cout << "[" << (ite->first.first) << ", " << (ite1->first.second)
             << ", "<< (ite->second) << "]";
        if (++ite1 != mp.end())
            cout << ",";
        else
            cout << endl;
        ite = --ite1;
    }
    return 0;
}



int main()
{

    char tmp[128];
    memset(tmp, 0, sizeof(tmp));
    char ch = 0;
    while (cin >> ch)
    {
        tmp[ch]++;
        if (tmp[ch] == 3)
        {
            cout << ch << endl;
            break;
        }
    }
    cout << int(tmp['P']) << endl;
    return 0;
}

bool check(char ch)
{
    return ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E';
}
int main()
{
    string str;
    char has = 0;

    int beg = 0, pre = -1, max = 0;
    while (cin >> str)
    {
        for (int i = 0; i < str.size(); ++i)
        {
            if (check(str[i]))
            {
                has |= (1 << (str[i] - 'A'));
                //第一个出现的位置
                if (pre == -1)
                {
                    beg = i;
                }
                else
                    max = max > (i - pre - 1) ? max : (i - pre - 1);
                pre = i;
            }
        }
        if (has != 31)
        {
            cout << 0 << endl;
        }
        else
        {
            max = max > (beg + str.size() - 1 - pre) ? max
                                                     : (beg + str.size() - 1 -
pre); cout << max << endl; cout << "pre" << pre << endl; cout << "beg" << beg <<
endl; cout << "size" << str.size() << endl;
        }
        has = 0;
        pre = -1;
    }
    return 0;
}
extern "C"
{
    char *p2 = "test2";
    //p2[0] = 'a';
}
int main()
{
    char *p1 = "test1.";
    cout << static_cast<const void *>(p1) << endl;
    cout << static_cast<const void *>(p2) << endl;
    cout << static_cast<const void *>("test1.") << endl;
    cout << static_cast<const void *>("test3.") << endl;
    //p1[0] = 'a';
    //p2[0] = 'a';
    int n = 0;
    vector<int> vi(n, 0);
    vector<int> dp(n + 1, 0x7fffffff);
    dp[0] = 0;
    for (auto &elem : vi)
        cin >> elem;
    for (int i = 0; i < n; ++i)
        for (int j = 1; j <= vi[i]; ++j)
        {
            if (i + j <= n)
                dp[i + j] = min(dp[i] + 1, dp[i + j]);
            else
                break;
        }
    if (dp.back() != 0x7fffffff)
        cout << dp.back() << endl;
    else
        cout << -1 << endl;
    return 0;
}
#include <bits/stdc++.h>
using namespace std;

int main() {
  vector<int> vi{153, 370, 371, 407};
  int n = 0, m = 0;
  while (cin >> m >> n) {
    auto low = lower_bound(vi.begin(), vi.end(), m);
    auto up = upper_bound(vi.begin(), vi.end(), n);

    low = (m <= vi[0]) ? low : low + 1;
    if (low == up)
      cout << "no" << endl;
    else {
      for (auto ite = low; ite != up; ++ite)
        cout << *ite << " ";
      cout << endl;
    }
  }
  return 0;
}

int main()
{
    int n = 0;
    cin >> n;
    int arr[26];
    int cnt[10] = {'G', 'I', 'Z', 'N', 'W', 'H', 'U', 'F', 'X', 'S'};
    memset(arr, 0, sizeof(arr));
    while (n--)
    {
        string str;
        cin >> str;
        for (int i = 0; i < str.size(); ++i)
        {
            arr[str[i] - 'A']++;
        }

        // 3的数目
        arr['H' - 'A'] -= arr['G' - 'A'];
        // 7
        arr['S' - 'A'] -= arr['X' - 'A'];
        // 5
        arr['F' - 'A'] -= arr['U' - 'A'];
        // 9
        arr['I' - 'A'] -= (arr['F' - 'A'] + arr['X' - 'A'] + arr['G' - 'A']);
        // 1
        arr['N' - 'A'] -= (arr['I' - 'A'] + arr['S' - 'A']);
        for (int i = 0; i < 10; ++i)
        {
            while (arr[cnt[i] - 'A']--)
                cout << i;
        }
        cout << endl;
        memset(arr, 0, sizeof(arr));
    }
    return 0;
}
int main() {
    int t;
    char n[10004];
    int b[10] = {0,0,0,0,0,0,0,0,0,0};
    int i,j,k,s;
    scanf("%d",&t);
    while(t--) {
        memset(b,0,sizeof(b));
        scanf("%s",&n);
        s = strlen(n);
        for(i = 0; i< s; i++) {
            if(n[i] == 'Z') b[0]++;//0对应2
            if(n[i] == 'O') b[1]++;
            if(n[i] == 'W') b[2]++;//2对应4
            if(n[i] == 'H') b[3]++;
            if(n[i] == 'U') b[4]++;//4对应6
            if(n[i] == 'F') b[5]++;
            if(n[i] == 'X') b[6]++;//6对应8
            if(n[i] == 'S') b[7]++;
            if(n[i] == 'G') b[8]++;//8对应0
            if(n[i] == 'I') b[9]++;
        }
        b[1] = b[1] - b[0] - b[2] - b[4];//1对应3
        b[3] = b[3] - b[8];//3对应5
        b[5] = b[5] - b[4];//5对应7
        b[7] = b[7] - b[6];//7对应9
        b[9] = b[9] - b[8] - b[5]- b[6];//9对应1
        while(b[8]--) {
            printf("%c",'0');
        }
        while(b[9]--) {
            printf("%c",'1');
        }
        while(b[0]--) {
            printf("%c",'2');
        }
        while(b[1]--) {
            printf("%c",'3');
        }
        while(b[2]--) {
            printf("%c",'4');
        }
        while(b[3]--) {
            printf("%c",'5');
        }
        while(b[4]--) {
            printf("%c",'6');
        }
        while(b[5]--) {
            printf("%c",'7');
        }
        while(b[6]--) {
            printf("%c",'8');
        }
        while(b[7]--) {
            printf("%c",'9');
        }
        printf("\n");
    }
}
#include "a.h"
int* fuck1(int& i)
{
    return &i;
}

int main()
{
    struct A tmp;
    const int a = 1;
    auto &a1 = a,  *a_ptr = &a;
    int b = 1,c=2;
    int *bPtr = &b;
    decltype(fuck1(b)) bPtr1 = bPtr;
    bPtr = &c;
    cout << *bPtr1 << endl;
    return 0;
}
class Solution
{
  public:
    int getSplitNode(vector<string> matrix, int indexA, int indexB)
    {
        if (matrix.empty())
            return 0;
        if (indexA == 0 || indexB == 0)
            return 0;
        vector<int> pathA, pathB;
        vector<bool> visitedA(matrix.size(), false), visitedB(matrix.size(),
false); visitedA[indexA] = true; visitedB[indexB] = true;
        pathA.push_back(indexA);
        pathB.push_back(indexB);
        findPath(matrix, pathA, visitedA);
        findPath(matrix, pathB, visitedB);
        for (auto &elem : pathA)
            cout << elem << "<-";
        cout << endl;
        for (auto &elem : pathB)
            cout << elem << "<-";
        cout << endl;
        int i = pathA.size() - 1, j = pathB.size() - 1;
        for (; i >= 0 && j >= 0; --i, --j)
        {
            if (pathA[i] != pathB[j])
                return pathA[i + 1];
        }
        //一方的路径已经走完了
        if (i < 0)
            return pathA[0];
        else
            return pathB[0];
    }

  private:
    bool findPath(const vector<string> &matrix, vector<int> &vi,
                  vector<bool> &visited)
    {
        //找到了到根节点的路径
        if (matrix[vi.back()][0] == '1')
        {
            vi.push_back(0);
            visited[0] = true;
            return true;
        }
        for (int i = 1; i < matrix.size(); ++i)
        {
            if (matrix[vi.back()][i]=='1' && visited[i] != true)
            {
                vi.push_back(i);
                visited[i] = true;
                if (findPath(matrix, vi, visited))
                    return true;
                vi.pop_back();
                // visited[i]=false;
            }
        }
        return false;
    }
};

struct  fuck
{
    bool cmp(const int a,const int b)  const
    {
        return a < b;
    }
    vector<int> vi;
    void func1()
    {
        sort(vi.begin(),vi.end(),this->cmp);
    }
};

int main()
{
    fuck f1;
    f1.func1();
    int a = 0;
    int &b = a;
    cout << static_cast<void *> (&a) << endl;
    cout << static_cast<void *> (&b) << endl;
    Solution s;
    vector<string> matrix{"00111000000", "00100000000", "11000001000",
                          "10000110000", "10000000010", "00010000100",
                          "00010000001", "00100000000", "00000100000",
                          "00001000000", "00000010000"};
    cout << s.getSplitNode(matrix, 9, 10) << endl;
    return 0;
}
#include "rbtree.h"

int main()
{
    RBTree<int, int, _Identity<int>, less<int>> rb1;
    cout << rb1.size() << endl;
    rb1.insert_unique(10);
    rb1.insert_unique(7);
    rb1.insert_unique(8);
    rb1.insert_unique(15);
    rb1.insert_unique(5);
    rb1.insert_unique(6);
    rb1.insert_unique(15);
    rb1.insert_unique(11);
    rb1.insert_unique(13);
    rb1.insert_unique(12);
    cout << rb1.size() << endl;
    rb1.delete_node(12);
    rb1.delete_node(13);
    rb1.delete_node(11);
    rb1.delete_node(6);
    cout << rb1.size() << endl;
    return 0;
}
void move(const int src,const int mid,const int dest,const int n)
{
    if(n==1)
        cout << "Move num:1 from " << src << " to " << dest << endl;
    else
    {
        move(src,dest,mid,n-1);
        cout << "Move num:"<<n<<" from " << src << " to " << dest << endl;
        move(mid,src,dest,n-1);
    }
}

int move2(const int src,const int mid,const int dest,const int n)
{
    if(n==1)
    {
        cout << "Move num:1 from " << src << " to " << mid << endl;
        cout << "Move num:1 from " << mid << " to " << dest << endl;
        return 2;
    }
    else
    {
        int n1=move2(src, mid, dest, n - 1);
        cout << "Move num:"<<n<<" from " << src << " to " << mid << endl;
        n1 += 1;
        int n2=move2(dest, mid, src, n - 1);
        cout << "Move num:"<<n<<" from " << mid << " to " << dest << endl;
        n2 += 1;
        int n3=move2(src, mid, dest, n - 1);
        return n1 + n2 + n3;
    }
}
int main()
{
    int n=move2(1, 2, 3, 2);
    cout << n << endl;

    return 0;
}
*/
class Solution {
public:
  /**
   * 计算你能获得的最大收益
   *
   * @param prices Prices[i]即第i天的股价
   * @return 整型
   */
  int calculateMax(vector<int> prices) {
    if (prices.empty() || prices.size() == 1)
      return 0;
    int max = 0, cur_min = prices[0];
    vector<int> dpl(prices.size(), 0), dpr(prices.size(), 0);
    // dpl
    for (int i = 1; i < prices.size(); ++i) {
      if (prices[i] < cur_min)
        cur_min = prices[i];
      if (max < prices[i] - cur_min)
        max = prices[i] - cur_min;
      dpl[i] = max;
    }
    // dpr
    max = 0;
    int cur_max = prices.back();
    for (int i = prices.size() - 2; i; --i) {
      if (prices[i] > cur_max)
        cur_max = prices[i];
      if (max < cur_max - prices[i])
        max = cur_max - prices[i];
      dpr[i] = max;
    }
    max = 0;
    for (int i = 0; i < prices.size(); ++i)
      if (max < dpl[i] + dpr[i])
        max = dpl[i] + dpr[i];
    for (auto &elem : dpl)
      cout << elem << " ";
    cout << endl;
    for (auto &elem : dpr)
      cout << elem << " ";
    return max;
  }
};
class Solution2 {
public:
  //就没有什么递推的规律吗。。。
  string countAndSay(int n) {
    if (n <= 0)
      return "";
    if (n == 1)
      return "1";
    string s("1");
    int cnt = 1;
    while (--n) {
      string tmp;
      for (int i = 1; i < s.size(); ++i) {
        if (s[i] == s[i - 1])
          cnt++;
        else {
          tmp += (to_string(cnt) + s[i - 1]);
          cnt = 1;
        }
      }
      tmp += (to_string(cnt) + s.back());
      s = tmp;
      cout << s << endl;
    }
    return s;
  }
};
// Forward declaration of isBadVersion API.
bool isBadVersion(int version) { return version >= 1702766719; }

class Solution8 {
public:
  //这不就是二分法？
  int firstBadVersion(int n) {
    if (n <= 0)
      return 0;
    long long l = 1, r = n;
    while (l <= r) {
      long long mid = (l + r) / 2;
      if (isBadVersion(mid))
        r = mid - 1;
      else
        l = mid + 1;
    }
    return l;
  }
};
class Solution9 {
public:
  uint32_t reverseBits(uint32_t n) {
    uint32_t ret = 0;
    for (int i = 0; i < 32; ++i) {
      if (n & (1 << i))
        ret += (1 << (31 - i));
    }
    return ret;
  }
};
class Solution90 {
public:
  void clearRow(vector<vector<int>> &matrix, const int row) {
    for (auto &elem : matrix[row])
      elem = 0;
  }

  void clearCol(vector<vector<int>> &matrix, const int col) {
    for (auto &elem : matrix)
      elem[col] = 0;
  }
  void setZeroes(vector<vector<int>> &matrix) {
    if (matrix.empty())
      return;
    // O(m+n)的解决方案，使用两个数组 一个存哪几行需要清零，一个存哪几列需要清零
    bool row[matrix.size()], col[matrix[0].size()];
    memset(row, 0, matrix.size() * sizeof(bool));
    memset(col, 0, matrix[0].size() * sizeof(bool));
    for (int i = 0; i < matrix.size(); ++i) {
      for (int j = 0; j < matrix[i].size(); ++j) {
        if (!matrix[i][j]) {
          row[i] = true;
          col[j] = true;
        }
      }
    }
    for (int i = 0; i < matrix.size(); ++i) {
      if (row[i])
        clearRow(matrix, row[i]);
    }

    for (int i = 0; i < matrix[0].size(); ++i) {
      if (col[i])
        clearCol(matrix, col[i]);
    }
  }
};
class Solution99 {
public:
  void dfs(vector<vector<char>> &grid, const int i, const int j) {
    grid[i][j] = '0';
    if (i > 0 && grid[i - 1][j] == '1')
      dfs(grid, i - 1, j);
    if (i < (grid.size() - 1) && grid[i + 1][j] == '1')
      dfs(grid, i + 1, j);
    if (j < (grid[0].size() - 1) && grid[i][j + 1] == '1')
      dfs(grid, i, j + 1);
    if (j > 0 && grid[i][j - 1] == '1')
      dfs(grid, i, j - 1);
  }
  //图的深度优先搜索和广度优先搜索。。忘了
  int numIslands(vector<vector<char>> &grid) {
    if (grid.empty())
      return 0;
    int ret = 0;
    for (int i = 0; i < grid.size(); ++i) {
      for (int j = 0; i < grid[0].size(); ++j) {
        if (grid[i][j] == '1') {
          ret++;
          dfs(grid, i, j);
        }
      }
    }
    return ret;
  }
};
class Solution1018 {
public:
  void help(const vector<vector<int>> &matrix, const int x1, const int x2,
            const int y1, const int y2, vector<int> &ret) {
    for (int i = y1; i <= y2; ++i) {
      ret.push_back(matrix[x1][i]);
    }
    if (ret.size() == matrix.size() * matrix[0].size())
      return;
    for (int i = x1 + 1; i <= x2; ++i) {
      ret.push_back(matrix[i][y2]);
    }
    if (ret.size() == matrix.size() * matrix[0].size())
      return;
    for (int i = y2 - 1; i >= y1; --i) {
      ret.push_back(matrix[x2][i]);
    }
    if (ret.size() == matrix.size() * matrix[0].size())
      return;
    for (int i = x2 - 1; i >= x1; --i) {
      ret.push_back(matrix[i][y1]);
    }
    if (ret.size() == matrix.size() * matrix[0].size())
      return;
    help(matrix, x1 + 1, x2 - 1, y1 + 1, y2 - 1, ret);
  }
  vector<int> spiralOrder(vector<vector<int>> &matrix) {
    if (matrix.empty() || matrix[0].empty())
      return vector<int>();
    int m = matrix.size(), n = matrix[0].size();
    vector<int> ret;
    help(matrix, 0, m - 1, 0, n - 1, ret);

    return ret;
  }
};
class Solution111 {
public:
  bool help(const vector<vector<int>> &board, const int i, const int j) {
    int cnt = 0;
    bool isDead = !board[i][j];
    if (i > 0 && board[i - 1][j])
      cnt++;
    if (i < (board.size() - 1) && board[i + 1][j])
      cnt++;
    if (j > 0 && board[i][j - 1])
      cnt++;
    if (j < (board[0].size() - 1) && board[i][j + 1])
      cnt++;
    if (i > 0 && j > 0 && board[i - 1][j - 1])
      cnt++;
    if (i < (board.size() - 1) && j > 0 && board[i + 1][j - 1])
      cnt++;
    if (j < (board[0].size() - 1) && i > 0 && board[i - 1][j + 1])
      cnt++;
    if (j < (board[0].size() - 1) && i < (board.size() - 1) &&
        board[i + 1][j + 1])
      cnt++;
    if (isDead) {
      if (cnt == 3)
        return true;
    } else {
      if (cnt < 2 || cnt > 3)
        return true;
    }
    return false;
  }
  void gameOfLife(vector<vector<int>> &board) {
    if (board.empty() || board[0].empty())
      return;
    //非原地算法
    vector<vector<bool>> toBeChanged(board.size(),
                                     vector<bool>(board[0].size(), false));
    for (int i = 0; i < board.size(); ++i)
      for (int j = 0; j < board[0].size(); ++j) {
        toBeChanged[i][j] = help(board, i, j);
      }

    for (int i = 0; i < board.size(); ++i)
      for (int j = 0; j < board[0].size(); ++j) {
        if (toBeChanged[i][j])
          board[i][j] = !board[i][j];
      }
  }
};
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};
bool cmp(ListNode *l1, ListNode *l2) { return l1->val < l2->val; }
struct CMP {
  bool operator()(ListNode *l1, ListNode *l2) { return l1->val < l2->val; }
};

class Solution136 {
public:
  ListNode *mergeKLists(vector<ListNode *> &lists) {
    if (lists.empty())
      return nullptr;
    /*
ListNode *ret=lists[0];
for(int i=1;i<lists.size();++i)
{
ret=merge2Lists(ret,lists[i]);
}
*/
    ListNode *ret = nullptr, *cur = nullptr;
    typedef bool (*f1_type)(ListNode *, ListNode *);
    priority_queue<ListNode *, vector<ListNode *>, f1_type> pq(cmp);
    for (int i = 0; i < lists.size(); ++i)
      if (lists[i])
        pq.push(lists[i]);
    while (!pq.empty()) {
      ListNode *node = pq.top();
      pq.pop();
      //最先弹出的节点即为头部
      if (!ret)
        ret = node;
      if (!cur)
        cur = node;
      else {
        cur->next = node;
        cur = node;
      }
      if (node->next)
        pq.push(node->next);
    }
    return ret;
  }
};
bool fuck(int a, int b) { return a == b; }
class Solution144 {
public:
  ListNode *merge(ListNode *l1, ListNode *l2, int n1, int n2) {
    if (!n1)
      return l2;
    if (!n2)
      return l1;
    ListNode *head = (l1->val > l2->val) ? l2 : l1, *ret = head;
    if (l1->val > l2->val)
      l2 = l2->next;
    else
      l1 = l1->next;
    while (n1 && n2) {
      if (l1->val > l2->val) {
        head->next = l2;
        head = head->next;
        l2 = l2->next;
        n2--;
      } else {
        head->next = l1;
        head = head->next;
        l1 = l1->next;
        n1--;
      }
    }
    while (n1) {
      head->next = l1;
      head = head->next;
      l1 = l1->next;
      n1--;
    }
    while (n2) {
      head->next = l2;
      head = head->next;
      l2 = l2->next;
      n2--;
    }
    return ret;
  }
  ListNode *mergeSort(ListNode *head, int n) {
    if (n == 1) {
      return head;
    }
    ListNode *l1 = mergeSort(head, n / 2);
    /*
ListNode *l1T=l1;
while(l1T)
{
cout<<l1T->val<<" ";
l1T=l1T->next;
}
cout<<endl;
*/
    int tmp = n / 2;
    while (tmp--) {
      head = head->next;
    }

    ListNode *l2 = mergeSort(head, n - n / 2);
    /*
ListNode *l2T=l2;
while(l2T)
{
cout<<l2T->val<<" ";
l2T=l2T->next;
}
cout<<endl;
*/
    return merge(l1, l2, n / 2, n - n / 2);
  }
  ListNode *sortList(ListNode *head) {
    if (!head || !head->next)
      return head;
    //原始解法:O(n2)
    //每轮遍历两次链表，第一次找最小值，第二次把最小值加入到新建的链表尾(就是一个头部)
    //要做到O(nlogn) 感觉应该是用归并排序了
    int n = 0;
    ListNode *cur = head;
    while (cur) {
      cur = cur->next;
      n++;
    }
    return mergeSort(head, n);
  }
};
class Solution153 {
public:
  //判断两个单词能否转换
  bool help(const string &s1, const string &s2) {
    int diff = 0;
    for (int i = 0; i < s1.size(); ++i) {
      if (s1[i] != s2[i])
        diff++;
      if (diff > 1)
        return false;
    }
    return true;
  }

  int ladderLength(string beginWord, string endWord, vector<string> &wordList) {
    if (wordList.empty())
      return 0;
    //这应该是无向图求两个点是否连通的问题
    // 1.首先判断endWord是否再wordList中
    // 2.然后判断beginWord能否先转化到字典中的单词
    bool flag1 = false, flag2 = false;
    vector<string> start;
    for (int i = 0; i < wordList.size(); ++i) {
      if (wordList[i] == endWord)
        flag1 = true;
      if (help(beginWord, wordList[i])) {
        if (!flag2)
          flag2 = true;
        start.push_back(wordList[i]);
      }
    }
    if (!flag1 || !flag2)
      return 0;
    // 3.然后生成邻接表
    unordered_map<string, vector<string>> um;
    um[beginWord] = start;
    for (int i = 0; i < wordList.size(); ++i) {
      for (int j = 0; j < wordList.size(); ++j) {
        if (j != i && help(wordList[i], wordList[j])) {
          um[wordList[i]].push_back(wordList[j]);
        }
      }
    }
    // 4.BFS寻找是否存在 且最短的转换长度
    queue<string> q;
    set<string> visited;
    q.push(beginWord);
    visited.insert(beginWord);
    int ret = 1;
    while (!q.empty()) {
      ret++;
      int size = q.size();
      while (size) {
        string tmp = q.front();
        for (auto &elem : um[tmp]) {
          if (visited.find(elem) == visited.end()) {
            visited.insert(elem);
            q.push(elem);
          }
          if (elem == endWord) {
            return ret;
          }
        }
        size--;
        q.pop();
      }
    }

    return 0;
  }
};
class Solution162 {
public:
  //判断两个单词能否转换
  bool help(const string &s1, const string &s2) {
    int diff = 0;
    for (int i = 0; i < s1.size(); ++i) {
      if (s1[i] != s2[i])
        diff++;
      if (diff > 1)
        return false;
    }
    return true;
  }

  int ladderLength(string beginWord, string endWord, vector<string> &wordList) {
    if (wordList.empty())
      return 0;
    //这应该是无向图求两个点是否连通的问题
    // 1.首先判断endWord是否再wordList中
    // 2.然后判断beginWord能否先转化到字典中的单词
    /*
bool flag1=false,flag2=false;
vector<string> start;
for(int i=0;i<wordList.size();++i)
{
if(wordList[i]==endWord)
flag1=true;
if(help(beginWord,wordList[i]))
{
if(!flag2)
flag2=true;
start.push_back(wordList[i]);
}
}
if(!flag1||!flag2)
return 0;
*/
    unordered_set<string> wordSet(wordList.begin(), wordList.end());
    if (wordSet.find(endWord) == wordSet.end())
      return 0;
    /*
//3.然后生成邻接表
unordered_map<string,vector<string>> um;
um[beginWord]=start;
for(int i=0;i<wordList.size();++i)
{
for(int j=0;j<wordList.size();++j)
{
if(j!=i&&help(wordList[i],wordList[j]))
{
um[wordList[i]].push_back(wordList[j]);
}
}
}
*/
    // 4.BFS寻找是否存在 且最短的转换长度
    queue<string> q;
    unordered_set<string> visited;
    q.push(beginWord);
    visited.insert(beginWord);
    int ret = 1;
    while (!q.empty()) {
      ret++;
      int size = q.size();
      while (size--) {
        string cur = q.front();
        /*
for(auto& elem:um[tmp])
{
if(visited.find(elem)==visited.end())
{
visited.insert(elem);
q.push(elem);
}
if(elem==endWord)
{
return ret;
}
}
*/
        /*
for(int i=0;i<cur.size();++i)
{
char ch = cur[i];
for(int j='a';j<='z';++j)
{
cur[i]=j;
if(cur==endWord)
return ret;
if(!wordSet.empty()&&wordSet.find(cur)!=wordSet.end())
{
visited.insert(cur);
wordSet.erase(cur);
}
}
cur[i] = ch;
}
*/
        unordered_set<string> tmp(wordSet);
        for (auto elem : wordSet) {
          cout << elem << endl;
          if (help(elem, cur)) {
            if (elem == endWord)
              return ret;
            visited.insert(elem);
            tmp.erase(elem);
            q.push(elem);
          }
        }
        wordSet = tmp;
        q.pop();
      }
    }

    return 0;
  }
};
class Solution174 {
public:
  //把访问过的o点全改为v表示已访问(边界的o不变)
  void dfs(vector<vector<char>> &board, const int i, const int j) {
    if (i == -1 || i == board.size() || j == -1 || j == board.size())
      return;
    if (board[i][j] != 'O')
      return;
    board[i][j] = 'V';
    dfs(board, i - 1, j);
    dfs(board, i + 1, j);
    dfs(board, i, j - 1);
    dfs(board, i, j + 1);
  }
  void solve(vector<vector<char>> &board) {
    if (board.empty() || board[0].empty())
      return;
    int row = board.size(), col = board[0].size();
    // dfs即可
    for (int i = 0; i < row; ++i) {
      if (board[i][0] == 'O')
        dfs(board, i, 0);
      if (board[i][col - 1] == 'O')
        dfs(board, i, col - 1);
    }

    for (int i = 0; i < col; ++i) {
      if (board[0][i] == 'O')
        dfs(board, 0, i);
      if (board[row - 1][i] == 'O')
        dfs(board, row - 1, i);
    }
    for (int i = 0; i < row; ++i)
      for (int j = 0; j < col; ++j) {
        if (board[i][j] == 'V')
          board[i][j] = 'O';
        else if (board[i][j] == 'O')
          board[i][j] = 'X';
      }
  }
};
class Solution179 {
public:
  int dfs(const vector<vector<int>> &matrix, vector<vector<int>> &maxLen, int i,
          int j) {
    //这个点如果之前已经经过了 那么他dfs得到的值就是最大值
    if (maxLen[i][j]) {
      return maxLen[i][j];
    }
    maxLen[i][j] = 1;
    if (i > 0 && !maxLen[i - 1][j] && matrix[i - 1][j] > matrix[i][j]) {
      maxLen[i][j] = 1 + dfs(matrix, maxLen, i - 1, j);
    }
    if (j > 0 && !maxLen[i][j - 1] && matrix[i][j - 1] > matrix[i][j]) {
      maxLen[i][j] = max(maxLen[i][j], 1 + dfs(matrix, maxLen, i, j - 1));
    }
    if (i < (matrix.size() - 1) && !maxLen[i + 1][j] &&
        matrix[i + 1][j] > matrix[i][j]) {
      maxLen[i][j] = max(maxLen[i][j], dfs(matrix, maxLen, i + 1, j) + 1);
    }
    if (j < (matrix[0].size() - 1) && !maxLen[i][j + 1] &&
        matrix[i][j + 1] > matrix[i][j]) {
      maxLen[i][j] = max(maxLen[i][j], 1 + dfs(matrix, maxLen, i, j + 1));
    }
    return maxLen[i][j];
  }
  int longestIncreasingPath(vector<vector<int>> &matrix) {
    if (matrix.empty() || matrix[0].empty())
      return 0;
    // vector<vector<bool>>
    // visited(matrix.size(),vector<bool>(matrix[0].size(),false));
    vector<vector<int>> maxLen(matrix.size(), vector<int>(matrix[0].size(), 0));
    int ret = 1;
    for (int i = 0; i < matrix.size(); ++i) {
      for (int j = 0; j < matrix[0].size(); ++j) {
        ret = max(ret, dfs(matrix, maxLen, i, j));
      }
      //
    }
    return ret;
  }
};
struct TrieNode {
  int wordCnt;
  TrieNode *children[26];
  TrieNode() : wordCnt(0) { memset(children, 0, sizeof(TrieNode *) * 26); }
};
class Trie {
private:
  TrieNode root;
  TrieNode *findNode(string s) {
    TrieNode *cur = &root;
    for (int i = 0; i < s.size(); ++i) {
      if (cur->children[s[i] - 'a']) {
        cur = cur->children[s[i] - 'a'];
      } else
        return nullptr;
    }
    return cur;
  }

public:
  Trie() {}

  void insert(string s) {
    TrieNode *cur = &root;
    for (int i = 0; i < s.size(); ++i) {
      if (!cur->children[s[i] - 'a']) {
        cur->children[s[i] - 'a'] = new TrieNode();
      }
      cur = cur->children[s[i] - 'a'];
      if (i == s.size() - 1)
        cur->wordCnt += 1;
    }
  }

  bool search(string s) {
    TrieNode *node = findNode(s);
    return node && node->wordCnt;
  }

  bool startsWith(string s) { return findNode(s); }
};
class Solution189 {
public:
  void help(const vector<vector<char>> &board, vector<vector<bool>> &visited,
            Trie &t, vector<string> &ret, string &tmp, const int i,
            const int j) {
    if (visited[i][j])
      return;
    visited[i][j] = true;
    tmp += board[i][j];
    //回溯提前终止
    if (!t.startsWith(tmp)) {
      visited[i][j] = false;
      tmp.erase(tmp.size() - 1, 1);
      return;
    }
    //找到了一个单词
    if (t.search(tmp)) {
      ret.push_back(tmp);
    }
    int row[] = {1, -1, 0, 0};
    int col[] = {0, 0, 1, -1};
    for (int k = 0; k < 4; ++k) {
      int n1 = i + row[k];
      int n2 = j + col[k];
      if (n1 >= 0 && n1 < board.size() && n2 >= 0 && n2 < board[0].size())
        help(board, visited, t, ret, tmp, n1, n2);
    }
    tmp.erase(tmp.size() - 1, 1);
    visited[i][j] = false;
  }
  vector<string> findWords(vector<vector<char>> &board, vector<string> &words) {
    if (words.empty() || board.empty() || board[0].empty())
      return vector<string>();
    Trie t;
    for (int i = 0; i < words.size(); ++i) {
      t.insert(words[i]);
    }
    vector<string> ret;
    string tmp;
    vector<vector<bool>> visited(board.size(),
                                 vector<bool>(board[0].size(), false));
    for (int i = 0; i < board.size(); ++i) {
      for (int j = 0; j < board.size(); ++j) {
        help(board, visited, t, ret, tmp, i, j);
      }
    }
    return ret;
  }
};
class Solution195 {
public:
  int help(const string &s, const string &p, const int sBegin, int pBegin) {
    //两者都遍历到底 说明匹配成功
    if (sBegin == s.size() && pBegin == p.size())
      return 2;
    //模式串到底但是字符串未到底 说明*通配的字符少了
    if (pBegin == p.size() && sBegin < s.size())
      return 1;
    //模式串没到底且后面不是*但是字符串到底了
    //这个如果返回的话就说明后面不需要用*匹配更多的字符了  因为肯定返回0
    if (sBegin == s.size() && pBegin < p.size() && p[pBegin] != '*')
      return 0;
    //对应字符相等
    if (s[sBegin] == p[pBegin] || p[pBegin] == '?')
      return help(s, p, sBegin + 1, pBegin + 1);
    else if (p[pBegin] == '*') {
      while (pBegin < p.size() - 1 && p[pBegin + 1] == '*')
        pBegin++;
      //当前*表示1个字符或不表示字符
      int flag = help(s, p, sBegin, pBegin + 1);
      if (flag != 1)
        return flag;
      //如果flag==0的话 即递归调用的结果是s到底，但是p没到底且后面不是*,那么
      //这里就没有必要再让当前pBegin这个*匹配更多的s字符了，这样返回的结果一定是0，
      //所以这里还需要一次剪枝
      return help(s, p, sBegin + 1, pBegin);
    }
    return 1;
  }
  bool isMatch(string s, string p) {
    if (s.empty() && p.empty())
      return true;
    return help(s, p, 0, 0);
  }
};
class Solution199 {
public:
  bool isMatch(string s, string p) {
    int index_s = 0;
    int index_p = 0;
    int star_p = -1;
    int mark_s = 0;
    while (index_s < s.size()) {
      //当前字符匹配
      if (s[index_s] == p[index_p] || p[index_p] == '?') {
        index_s++;
        index_p++;
      } else if (p[index_p] == '*') {
        //出现'*'，记录当前位置
        star_p = index_p;
        mark_s = index_s;
        index_p++;
      } else if (star_p != -1) {
        //不匹配且之前出现'*'时，回溯
        index_p = star_p + 1;
        index_s = mark_s + 1;
        mark_s++;
      } else {
        //不匹配且无'*'出现时，返回false
        return false;
      }
    }
    while (index_p < p.size()) {
      if (p[index_p] == '*')
        index_p++;
      else
        return false;
    }
    return true;
  }
};
class Solution204 {
public:
  void dfs(string s, vector<string> &ret, const int last_i, const int last_j,
           char ch) {
    int cnt = 0;
    for (int i = last_i; i < s.size(); ++i) {
      if (s[i] != '(' && s[i] != ')')
        continue;
      s[i] == ch ? cnt-- : cnt++;
      if (cnt == -1) {
        for (int j = last_j; j <= i; ++j) {
          if (s[j] == ch && (j == i || s[j + 1] != ch))
            dfs(s.substr(1, j) + s.substr(j + 1), ret, i, j, ch);
        }
        return;
      }
    }
    reverse(s.begin(), s.end());
    // ch=='('时表示这个dfs已经完成了
    if (ch == '(')
      ret.push_back(s);
    else
      dfs(s, ret, 0, 0, '(');
  }
  vector<string> removeInvalidParentheses(string s) {
    if (s.empty())
      return vector<string>(1, "");
    vector<string> ret;
    dfs(s, ret, 0, 0, ')');
    return ret;
  }
};
class Solution207 {
public:
  int help(const string &s, const string &p, const int sCur, const int pCur) {
    if (sCur == s.size() && pCur == p.size())
      return 2;
    if (pCur == p.size() && sCur < s.size())
      return 1;
    if (sCur == s.size() && p[pCur] != '*')
      return 0;
    if (pCur == p.size() - 1 || p[pCur + 1] != '*') {
      if (s[sCur] == p[pCur] || p[pCur] == '.')
        return help(s, p, sCur + 1, pCur + 1);
      else
        return 1;
    } else {
      int tmp = 1;
      while (pCur < p.size() - tmp && p[pCur + tmp] == '*')
        tmp++;
      if (s[sCur] == p[pCur] || p[pCur] == '.') {
        // i代表pCur指向的字符重复的次数
        for (int i = 0; i <= s.size() - sCur; ++i) {

          if (s[sCur + i] == p[pCur] || p[pCur] == '.') {
            int ret = help(s, p, sCur + i, pCur + tmp);
          } else
            return 1;
        }
      } else {
        return help(s, p, sCur, pCur + tmp);
      }
    }
  }
  bool isMatch(string s, string p) { return help(s, p, 0, 0) > 1; }
};
int partition(vector<int> &vi, const int l, const int r) {
  //...通过某种方式取得了p
  swap(vi[(l + r) / 2], vi[l]);
  int i = l;
  for (int j = l + 1; j <= r; ++j) {
    if (vi[j] <= vi[l]) {
      i++;
      swap(vi[i], vi[j]);
    }
  }
  swap(vi[i], vi[l]);
  return i;
}
int findKth(vector<int> &vi, const int l, const int r, const int k) {
  int ret = partition(vi, l, r);
  if (ret == k - 1)
    return vi[ret];
  if (ret < k - 1)
    return findKth(vi, ret + 1, r, k);
  return findKth(vi, l, r - 1, k);
}
void qSort(vector<int> &vi, const int l, const int r) {
  int ret = 0;
  if (l < r) {
    ret = partition(vi, l, r);
    qSort(vi, l, ret - 1);
    qSort(vi, ret + 1, r);
  }
}
class Solution215 {
public:
  int maxProfitNoLimit(const vector<int> &prices) {
    int ret = 0;
    for (int i = 0; i < prices.size() - 1; ++i) {
      if (prices[i + 1] > prices[i])
        ret += prices[i + 1] - prices[i];
    }
    return ret;
  }

  int maxProfit(int k, vector<int> &prices) {
    if (prices.empty())
      return 0;
    if (k >= prices.size() / 2)
      return maxProfitNoLimit(prices);
    vector<vector<int>> dp(prices.size(), vector<int>(k + 1, 0));
    for (int i = 1; i < prices.size(); ++i) {
      for (int j = 1; j <= k; ++j) {
        dp[i][j] =
            max(dp[i - 1][j], dp[i - 1][j - 1] + prices[i] - prices[i - 1]);
      }
    }
    return dp.back()[k];
  }
};
class Solution219 {
public:
  bool wordBreak(string s, const vector<string> &wordDict) {
    if (wordDict.empty())
      return s.empty();
    if (s.empty())
      return false;
    unordered_set<string> uss(wordDict.begin(), wordDict.end());
    //这个如果不说是动态规划我肯定想不到用动态规划
    //肯定就是回溯法走起了 这个摆明了回溯法好写代码。。。。
    // dp[i]=true if
    // dp[j]==true&&wordDict.find(s.substr(j,i-j+1))!=wordDict.end()
    vector<bool> dp(s.size() + 1, false);
    for (int i = 0; i < s.size(); ++i) {
      if (uss.find(s.substr(0, i + 1)) != uss.end()) {
        dp[i] = true;
        continue;
      }
      for (int j = 0; j < i; ++j) {
        if (!dp[i])
          dp[i] =
              (dp[j] == true && uss.find(s.substr(j + 1, i - j)) != uss.end());
        else
          break;
      }
    }
    return dp.back();
  }
};
class Solution222 {
public:
  int maxCoins(vector<int> &nums) {
    if (nums.empty())
      return 0;
    if (nums.size() == 1)
      return nums[0];
    //完全没思路
    //这个必须要用二维的dp来表示了
    //记dp[i][j]为区间i-j内戳爆气球能获得的最大利润
    //记k为最后i-j内最后一个戳爆的气球
    //有dp[i][j]=max(dp[i][j],nums[i-1]*nums[k]*nums[j+1]+dp[i][k-1]+dp[k+1][j])
    vector<vector<int>> dp(nums.size(), vector<int>(nums.size(), 0));
    for (int i = 0; i < nums.size(); ++i) {
      if (i == 0)
        dp[i][i] = nums[0] * nums[1];
      else if (i == dp.size() - 1)
        dp[i][i] = nums.back() * *(nums.end() - 2);
      else
        dp[i][i] = nums[i] * nums[i - 1] * nums[i + 1];
    }
    for (int i = 0; i < nums.size() - 1; ++i) {
      for (int j = i + 1; j < nums.size(); ++j) {
        for (int k = i; k <= j; ++k) {
          dp[i][j] =
              max(dp[i][j], nums[k] * (i == 0 ? 1 : nums[i - 1]) *
                                    (j == nums.size() - 1 ? 1 : nums[j + 1]) +
                                (k == 0 ? 0 : dp[i][k - 1]) +
                                (k == nums.size() - 1 ? 0 : dp[k + 1][j]));
        }
      }
    }
    return dp[0][nums.size() - 1];
  }
};
class Solution225 {
public:
  vector<pair<int, int>> getSkyline(vector<vector<int>> &buildings) {
    //每个矩形的左右顶点是可能产生关键点的点(不一定是这个点,但肯定在这个点纵轴所在的直线上)
    //思路:
    // 1.先将所有左右顶点按照x升序、y升序的顺序排序；
    // 2.遍历排序后的顶点序列,并使用一个结构记录当前遍历到的各个矩形的高:
    // a.遇到左顶点说明当前遇到一个新的矩形,将这个矩形的高压入；
    // b.遇到右顶点说明一个矩形结束了，把这个矩形的高删除；
    //经过上面的操作，如果记录高的结构中最大的高发生了变化(添加后变化说明遇到了上升沿，删除后变化说明遇到了下降沿)，说明需要记录下一个关键点
    // note:这里采用set来作为保存高的结构，为什么不用最大堆是因为最大堆无法找到任意一个非最大值的高；
    if (buildings.empty())
      return {};
    vector<pair<int, int>> points(buildings.size() * 2);
    for (int i = 0; i < buildings.size(); ++i) {
      //把左顶点的高置为负数用于区分左右定点
      points[2 * i] = make_pair(buildings[i][0], -buildings[i][2]);
      points[2 * i + 1] = make_pair(buildings[i][1], buildings[i][2]);
    }
    sort(points.begin(), points.end(),
         [](const pair<int, int> &p1, const pair<int, int> &p2) {
           return p1.first < p2.first ||
                  (p1.first == p2.first && p1.second < p2.second);
         });
    int prevMax = 0;
    multiset<int> heights;
    //地平线
    heights.insert(0);
    vector<pair<int, int>> ret;
    //遍历各个排序后的点
    for (auto &p : points) {
      if (p.second < 0)
        heights.insert(-p.second);
      else
        heights.erase(heights.find(p.second));
      int curMax = *heights.rbegin();
      if (curMax != prevMax) {
        //如何处理矩形边界线重合的问题
        ret.emplace_back(p.first, curMax);
        prevMax = curMax;
      }
    }
    return ret;
  }
};
class Solution230 {
public:
  vector<int> findDiagonalOrder(vector<vector<int>> &matrix) {
    //共m+n-1次
    if (matrix.empty() || matrix[0].empty())
      return vector<int>();
    int xBegin = 0, yBegin = 0, m = matrix.size(), n = matrix[0].size();
    vector<int> ret;
    ret.reserve(m * n);
    for (int i = 1; i <= m + n; ++i) {
      if (i % 2) {
        if (i <= m) {
          xBegin = i - 1;
          yBegin = 0;
        } else {
          xBegin = m - 1;
          yBegin = i - m;
        }
      } else {
        if (i <= n) {
          xBegin = 0;
          yBegin = i - 1;
        } else {
          xBegin = i - n;
          yBegin = n - 1;
        }
      }
      while (xBegin >= 0 && xBegin < m && yBegin >= 0 && yBegin < n) {
        ret.push_back(matrix[xBegin][yBegin]);
        if (i % 2) {
          xBegin -= 1;
          yBegin += 1;
        } else {
          xBegin += 1;
          yBegin -= 1;
        }
      }
    }
    return ret;
  }
};
class Solution235 {
public:
  string addBinary(string a, string b) {
    bool flag = false;
    string ret;
    int i = a.size() - 1, j = b.size() - 1;
    for (; i >= 0 && j >= 0; --i, --j) {
      short tmp = a[i] + b[j] - '0' - '0' + flag;
      if (tmp >= 2) {
        flag = true;
        if (tmp == 3)
          ret = "1" + ret;
        else
          ret += "0" + ret;
      } else {
        flag = false;
        ret = to_string(tmp) + ret;
      }
    }
    while (i >= 0) {
      short tmp = a[i] - '0' + flag;
      if (tmp == 2) {
        flag = true;
        ret = "0" + ret;
      } else {
        flag = false;
        ret = to_string(tmp) + ret;
      }
      i--;
    }
    while (j >= 0) {
      short tmp = b[j] - '0' + flag;
      if (tmp == 2) {
        flag = true;
        ret = "0" + ret;
      } else {
        flag = false;
        ret = to_string(tmp) + ret;
      }
      j--;
    }
    if (flag)
      ret = "1" + ret;
    return ret;
  }
};
class Solution241 {
public:
  /*
  O(n2),计算每个从i到j的和即可 最后返回一个最小的长度即可;
  
  */
  int minSubArrayLen(int s, const vector<int> &nums) {
    if (nums.empty())
      return 0;
    vector<int> dp(nums.size() + 1, 0);
    for (int i = 1; i < dp.size(); ++i)
      dp[i] += dp[i - 1] + nums[i - 1];
    int slow = 0, fast = 1;
    int ret = INT_MAX;
    for (; fast < dp.size();) {
      if (dp[fast] - dp[slow] >= s) {
        ret = min(ret, fast - slow);
        if (ret == 1)
          break;
        slow++;
      } else
        fast++;
    }
    return ret == INT_MAX ? 0 : ret;
  }
};
class Solution244 {
public:
  /*
  dp+滚动数组节省空间;
  */
  vector<int> getRow(int rowIndex) {
    if (rowIndex == 0)
      return {1};
    if (rowIndex == 1)
      return {1, 1};
    vector<int> ret(rowIndex + 1, 0);
    int tmp = 1;
    ret[0] = ret[1] = 1;
    for (int curIndex = 2; curIndex <= rowIndex; ++curIndex) {
      for (int i = 0; i <= curIndex; ++i) {
        if (i == 0 || i == curIndex)
          ret[i] = 1;
        else {
          tmp = ret[i];
          ret[i] += ret[i - 1];
        }
      }
      tmp = 1;
    }
    return ret;
  }
};
class Solution227 {
public:
  /*
  brute
  force解法：通过istringstream和getline来获取每个单词并保存，然后逆序构造句子。时间复杂度:O(n),空间复杂度:O(n)
  */
  void reverseWords(string &s) {
    if (s.size() < 2)
      return;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    [](int ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
    if (s.empty())
      return;
    istringstream iss(s);
    string str;
    // iss>>s;
    vector<string> vs;
    while (iss >> str)
      vs.push_back(str);
    str = "";
    for (int i = vs.size() - 1; i >= 0; --i) {
      str += vs[i];
      if (i > 0)
        str += " ";
    }
    s.swap(str);
    if (s[0] == ' ')
      s = "";
  }
  /*
  void reverseWords(string &s) {
    istringstream is(s);
    string tmp;
    is >> s;
    while (is >> tmp)
      s = tmp + " " + s;
    if (s[0] == ' ')
      s = "";
}
*/
};
class Trival {
public:
  int num;
  Trival() : num(-1) { cout << "default ctor called." << endl; }
  Trival(const int n) : num(n) {
    cout << "No args constructor called." << endl;
  }
  Trival(const Trival &t) {
    cout << this->num << "  Copy ctor called." << endl;
  }
  Trival &operator=(const Trival &t) {
    cout << this->num << "  Assign called." << endl;
    return *this;
  }
  ~Trival() { cout << this->num << "  Deconstructor called." << endl; }
};
Trival func() {
  Trival t(2);
  return t;
}
Trival func2(Trival &t) { return t; }
class Solution234 {
public:
  /*
    (not finished)知道要用栈，但是规则没理清楚;
    */
  /*
  string decodeString(string s) {
    stack<int> si;
    stack<string> ss;
    int num = 0;
    // str保存的是当前的结果 或者是某个[]中的字符串
    string str;
    for (int i = 0; i < s.size(); i++) {
      if (isdigit(s[i]))
        num = 10 * num + s[i] - '0';
      else if (isalpha(s[i]))
        str += s[i];
      else if (s[i] == '[') {
        si.push(num);
        num = 0;
        ss.push(str);
        str.clear();
      } else if (s[i] == ']') {
        int cur = si.top();
        si.pop();
        while (cur--)
          ss.top() += str;
        str = ss.top();
        ss.pop();
      }
    }
    return ss.empty() ? str : ss.top();
  }
  */
  string decodeString(const string &s, int &curIndex) {
    string ret;
    int num = 0;
    while (curIndex < s.size() && s[curIndex] != ']') {
      if (isalpha(s[curIndex]))
        ret += s[curIndex];
      else if (isdigit(s[curIndex]))
        num = 10 * num + s[curIndex] - '0';
      else if (s[curIndex] == '[') {
        curIndex++;
        string tmp = decodeString(s, curIndex);
        while (num--)
          ret += tmp;
      }
      curIndex++;
    }
    return ret;
  }
  string decodeString(string s) {
    /*
        stack<int> si;
        //stack<string> ss;
        int num=0;
        //str保存的是当前的结果
        string str;
        string tmp;
        for(int i=0;i<s.size();i++)
        {
            if(isdigit(s[i]))
                num=10*num+s[i]-'0';
            else if(isalpha(s[i]))
                str+=s[i];
            else if(s[i]=='[')
            {
                si.push(num);
                num=0;
                //ss.push(str);
                tmp=str;
                str.clear();
            }
            else if(s[i]==']')
            {
                int cur=si.top();
                si.pop();
                while(cur--)
                    //ss.top()+=str;
                    tmp+=str;
                //str=ss.top();
                //ss.pop();
                str=tmp;
                tmp.clear();
            }
        }
        //return ss.empty()?str:ss.top();
        return str;
        */
    int index = 0;
    decodeString(s, index);
  }
};

struct Base {
  Base() { std::cout << "  Base::Base()\n"; }
  // Note: non-virtual destructor is OK here
  ~Base() { std::cout << "  Base::~Base()\n"; }
};

struct Derived : public Base {
  Derived() { std::cout << "  Derived::Derived()\n"; }
  ~Derived() { std::cout << "  Derived::~Derived()\n"; }
};

void thr(std::shared_ptr<Base> p) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::shared_ptr<Base> lp = p; // thread-safe, even though the
                                // shared use_count is incremented
  {
    static std::mutex io_mutex;
    std::lock_guard<std::mutex> lk(io_mutex);
    std::cout << "local pointer in a thread:\n"
              << "  lp.get() = " << lp.get()
              << ", lp.use_count() = " << lp.use_count() << '\n';
  }
}
class Request
{
public:
  void process() // __attribute__ ((noinline))
  {
    std::lock_guard<mutex> lock(mutex_);
    print();
  }

  void print() const // __attribute__ ((noinline))
  {
    std::lock_guard<mutex> clock(mutex_);
  }

private:
  mutable std::mutex mutex_;
};

int main() {
  Request r;
  r.process();
 /*
  std::shared_ptr<Base> p = std::make_shared<Derived>();

  std::cout << "Created a shared Derived (as a pointer to Base)\n"
            << "  p.get() = " << p.get()
            << ", p.use_count() = " << p.use_count() << '\n';
  std::thread t1(thr, p), t2(thr, p), t3(thr, p);
  p.reset(); // release ownership from main
  std::cout << "Shared ownership between 3 threads and released\n"
            << "ownership from main:\n"
            << "  p.get() = " << p.get()
            << ", p.use_count() = " << p.use_count() << '\n';
  t1.join();
  t2.join();
  t3.join();
  std::cout << "All threads completed, the last one deleted Derived\n";
  */
  getchar();
}