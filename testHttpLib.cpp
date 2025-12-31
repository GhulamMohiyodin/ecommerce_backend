#include "httplib.h"
#include "nlohmann/json.hpp"

#include "Product.h"
#include "BTree.h"
#include "HashByID.h"

#include <fstream>
#include <limits>

using namespace httplib;
using json = nlohmann::json;

/* ================= FILE HANDLING ================= */

void loadFromFile(BTree& tree, HashByID& hashById)
{
    std::ifstream file("ProductDatabase.txt");
    if (!file) return;

    std::string line;
    while (true)
    {
        if (!getline(file, line)) break;
        if (line == "#") break;
        if (line.empty()) continue;

        int id = stoi(line);
        std::string name;
        getline(file, name);

        int price = 0, stock = 0;
        file >> price >> stock;
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        Product* p = new Product(id, name, price, stock);
        tree.insert(p);
        hashById.insert(id);
    }
}

void saveNode(BTreeNode* node, std::ofstream& file)
{
    if (!node) return;

    int i;
    for (i = 0; i < node->n; ++i)
    {
        if (!node->leaf && node->C[i])
            saveNode(node->C[i], file);

        Product* p = node->vals[i];
        if (p)
        {
            file << p->GetProductID() << "\n";
            file << p->GetName() << "\n";
            file << p->GetPrice() << "\n";
            file << p->GetStock() << "\n";
        }
    }
    if (!node->leaf && node->C[i])
        saveNode(node->C[i], file);
}

void saveToFile(BTree& tree)
{
    std::ofstream file("ProductDatabase.txt", std::ios::trunc);
    if (!file) return;

    if (tree.GetRoot())
        saveNode(tree.GetRoot(), file);

    file << "#\n";
}

void collectProducts(BTreeNode* node, std::vector<Product*>& products)
{
    if (!node) return;

    int i;
    for (i = 0; i < node->n; ++i)
    {
        if (!node->leaf && node->C[i])
            collectProducts(node->C[i], products);

        if (node->vals[i])
            products.push_back(node->vals[i]);
    }

    if (!node->leaf && node->C[i])
        collectProducts(node->C[i], products);
}

/* ================= CORS HELPER ================= */

void addCORS(Response& res) {
    res.set_header("Access-Control-Allow-Origin", "http://localhost:4200");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}


/* ================= MAIN BACKEND ================= */

int main()
{
    BTree tree(3);
    HashByID hashById(101);

    loadFromFile(tree, hashById);

    Server svr;

    // OPTIONS route for preflight
    svr.Options(".*", [&](const Request& req, Response& res) {
        addCORS(res);
        res.status = 200;
        });

    /* -------- ADD PRODUCT -------- */
    svr.Post("/product/add", [&](const Request& req, Response& res)
        {
            addCORS(res);  // ✅ CORS header added

            json body = json::parse(req.body);
            int id = body["id"];

            if (hashById.contains(id))
            {
                res.set_content("{\"error\":\"Product already exists\"}", "application/json");
                return;
            }

            Product* p = new Product(
                id,
                (std::string)body["name"],
                (int)body["price"],
                (int)body["stock"]
            );

            tree.insert(p);
            hashById.insert(id);
            saveToFile(tree);   // ✅ FIX

            res.set_content("{\"status\":\"Product added\"}", "application/json");
        });

    /* -------- DELETE PRODUCT -------- */
    svr.Delete("/product/delete", [&](const Request& req, Response& res)
        {
            addCORS(res);  // ✅ CORS header added

            json body = json::parse(req.body);
            int id = body["id"];

            if (!hashById.contains(id))
            {
                res.set_content("{\"error\":\"Product not found\"}", "application/json");
                return;
            }

            tree.remove(id);
            hashById.remove(id);
            saveToFile(tree);   // ✅ FIX

            res.set_content("{\"status\":\"Product deleted\"}", "application/json");
        });

    /* -------- UPDATE PRODUCT -------- */
    svr.Put("/product/update", [&](const Request& req, Response& res)
        {
            addCORS(res);  // ✅ CORS header added

            json body = json::parse(req.body);
            int id = body["id"];

            Product* p = tree.search(id);
            if (!p)
            {
                res.set_content("{\"error\":\"Product not found\"}", "application/json");
                return;
            }

            if (!body["name"].dump().empty())
                p->SetName((std::string)body["name"]);
            if ((int)body["price"] > 0)
                p->SetPrice((int)body["price"]);
            if ((int)body["stock"] >= 0)
                p->SetStock((int)body["stock"]);

            saveToFile(tree);   // ✅ FIX
            res.set_content("{\"status\":\"Product updated\"}", "application/json");
        });

    /* -------- BUY PRODUCT -------- */
    svr.Post("/product/buy", [&](const Request& req, Response& res)
        {
            addCORS(res);  // ✅ CORS header added

            json body = json::parse(req.body);
            int id = body["id"];
            int qty = body["quantity"];

            Product* p = tree.search(id);
            if (!p || qty <= 0 || p->GetStock() < qty)
            {
                res.set_content("{\"error\":\"Invalid purchase\"}", "application/json");
                return;
            }

            p->BuyProduct(qty);
            saveToFile(tree);   // ✅ FIX

            res.set_content("{\"status\":\"Purchase successful\"}", "application/json");
        });

    svr.Get("/products", [&](const Request&, Response& res)
        {
            addCORS(res);  // ✅ CORS header added

            std::vector<Product*> products;
            collectProducts(tree.GetRoot(), products);

            json result;   // object-based JSON

            int index = 0;
            for (auto* p : products)
            {
                json item;
                item["id"] = p->GetProductID();
                item["name"] = p->GetName();
                item["price"] = p->GetPrice();
                item["stock"] = p->GetStock();

                result[std::to_string(index)] = item;
                index++;
            }

            res.set_content(result.dump(), "application/json");
        });


    /* -------- SEARCH PRODUCT -------- */
    svr.Get("/product/search", [&](const Request& req, Response& res)
        {
            addCORS(res);  // ✅ CORS header added

            int id = stoi(req.get_param_value("id"));
            Product* p = tree.search(id);

            if (!p)
            {
                res.set_content("{\"error\":\"Product not found\"}", "application/json");
                return;
            }

            json out;
            out["id"] = p->GetProductID();
            out["name"] = p->GetName();
            out["price"] = p->GetPrice();
            out["stock"] = p->GetStock();

            res.set_content(out.dump(), "application/json");
        });

    svr.listen("0.0.0.0", 8080);
    return 0;
}
