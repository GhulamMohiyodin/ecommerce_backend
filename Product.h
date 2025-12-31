#pragma once
#include<iostream>
#include<iomanip>
using namespace std;
class Product
{
private:
	int ProductID;
	string ProductName;
	int Price;
	int Stock;
public:
	Product(int id, string name, int p, int s);
	void DisplayProduct();
	int GetPrice();
	int GetStock();
	int GetProductID();
	string GetName();
	void SetName(string name);
	void SetPrice(int p);
	void SetStock(int s);
	void BuyProduct(int quantity);
	~Product();
};

