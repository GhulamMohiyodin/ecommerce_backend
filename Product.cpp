#include "Product.h"
#include "Product.h"


Product::Product(int id, string name, int p, int s) : ProductID(id), ProductName(name), Price(p), Stock(s) {}

void Product::DisplayProduct()
{
	cout << ProductID << "\t" << left << setw(20) << ProductName << "\t\t" << Price << "\t" << Stock << endl;
}
int Product::GetPrice()
{
	return Price;
}

int Product::GetStock()
{
	return Stock;
}
int Product::GetProductID()
{
	return ProductID;
}
void Product::BuyProduct(int quantity)
{
	if (quantity <= Stock) {
		Stock -= quantity;
	}
	else {
		cout << "Insufficient stock!" << endl;
	}
}

string Product::GetName()
{
	return ProductName;
}
void Product::SetName(string name)
{
	ProductName = name;
}
void Product::SetPrice(int p)
{
	Price = p;
}
void Product::SetStock(int s)
{
	Stock = s;
}

Product::~Product() {}