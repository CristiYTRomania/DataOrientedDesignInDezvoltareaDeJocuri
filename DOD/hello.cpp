#include<iostream>
#include<vector>
#include<climits>
#include<complex>
#include<iomanip>
#include<iterator>
#include<algorithm>
using namespace std;
int main()
{
	vector<complex<long double>> v;
	complex<long double>sum = 0;
	vector<complex<long double>>::iterator it;
	v.push_back((long double)0);
	v.push_back((long double)LLONG_MAX);
	size_t n = v.size(); /// size_t = unsigned long long
	for (auto& x : v)
		x += (long double)LLONG_MAX;
	for (auto x : v)
		cout << fixed << x << ' ';
	reverse(v.begin(), v.end());
	cout << endl;
	for (it = v.begin(); it != v.end(); it++)
	{
		cout << *it << ' ';
		sum += *it;
	}
	cout << endl;
	cout << "Hello world!" << endl;
	cout << sum / (long double)n << endl;
	cout << sum / (long double)0 << endl;
	cout << (sum * (long double)0.0) / (long double)0 << endl;
	cout << ((long double)0 == (long double)0) << endl;
	cout << *find(v.begin(), v.end(), (long double)LLONG_MAX) << endl;
	cout << (find(v.begin(), v.end(), (long double)LLONG_MAX)  != v.end());
	
	return 0;
}