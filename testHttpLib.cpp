#include "httplib.h"
#include "nlohmann/json.hpp"
#include "Product.h"
#include "BTree.h"
#include "HashByID.h"
#include <fstream>
#include <limits>
#include <vector>

using namespace httplib;
using json = nlohmann::json;

/* ================= FILE HANDLING ================= */

void loadFromFile(BTree& tree, HashByID& hashById)
{
    std::ifstream file("ProductDatabase.txt");
    if (!file) return;

    std::string line;
    while (getline(file, line))
    {
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

/* ================= CORS ================= */

void addCORS(Response& res)
{
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

/* ================= MAIN ================= */

int main()
{
    BTree tree(3);
    HashByID hashById(101);
    loadFromFile(tree, hashById);

    Server svr;

    svr.Options(".*", [](const Request&, Response& res) {
        addCORS(res);
        res.status = 200;
        });

    /* -------- ADD PRODUCT -------- */
    svr.Post("/product/add", [&](const Request& req, Response& res) {
        addCORS(res);

        json body = json::parse(req.body, nullptr, false);
        if (!body.contains("id") || !body.contains("name") ||
            !body.contains("price") || !body.contains("stock"))
        {
            res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
            return;
        }

        int id = body["id"];
        if (hashById.contains(id))
        {
            res.set_content("{\"error\":\"Product already exists\"}", "application/json");
            return;
        }

        Product* p = new Product(
            id,
            body["name"].get<std::string>(),
            body["price"],
            body["stock"]
        );

        tree.insert(p);
        hashById.insert(id);
        saveToFile(tree);

        res.set_content("{\"status\":\"Product added\"}", "application/json");
        });

    /* -------- DELETE PRODUCT -------- */
    svr.Delete("/product/delete", [&](const Request& req, Response& res) {
        addCORS(res);

        json body = json::parse(req.body, nullptr, false);
        if (!body.contains("id"))
        {
            res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
            return;
        }

        int id = body["id"];
        if (!hashById.contains(id))
        {
            res.set_content("{\"error\":\"Product not found\"}", "application/json");
            return;
        }

        tree.remove(id);
        hashById.remove(id);
        saveToFile(tree);

        res.set_content("{\"status\":\"Product deleted\"}", "application/json");
        });

    /* -------- UPDATE PRODUCT -------- */
    svr.Put("/product/update", [&](const Request& req, Response& res) {
        addCORS(res);

        json body = json::parse(req.body, nullptr, false);
        if (!body.contains("id"))
        {
            res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
            return;
        }

        int id = body["id"];
        if (!hashById.contains(id))
        {
            res.set_content("{\"error\":\"Product not found\"}", "application/json");
            return;
        }

        Product* p = tree.search(id);
        if (!p)
        {
            res.set_content("{\"error\":\"Product not found\"}", "application/json");
            return;
        }

        if (body.contains("name"))  p->SetName(body["name"]);
        if (body.contains("price")) p->SetPrice(body["price"]);
        if (body.contains("stock")) p->SetStock(body["stock"]);

        saveToFile(tree);
        res.set_content("{\"status\":\"Product updated\"}", "application/json");
        });

    /* -------- BUY PRODUCT -------- */
    svr.Post("/product/buy", [&](const Request& req, Response& res) {
        addCORS(res);

        json body = json::parse(req.body, nullptr, false);
        if (!body.contains("id") || !body.contains("quantity"))
        {
            res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
            return;
        }

        int id = body["id"];
        int qty = body["quantity"];

        if (!hashById.contains(id))
        {
            res.set_content("{\"error\":\"Product not found\"}", "application/json");
            return;
        }

        Product* p = tree.search(id);
        if (!p || qty <= 0 || p->GetStock() < qty)
        {
            res.set_content("{\"error\":\"Invalid purchase\"}", "application/json");
            return;
        }

        p->BuyProduct(qty);
        saveToFile(tree);

        res.set_content("{\"status\":\"Purchase successful\"}", "application/json");
        });

    /* -------- GET ALL PRODUCTS -------- */
    svr.Get("/products", [&](const Request&, Response& res) {
        addCORS(res);

        std::vector<Product*> products;
        collectProducts(tree.GetRoot(), products);

        json result = json::array();
        for (auto* p : products)
        {
            result.push_back({
                {"id", p->GetProductID()},
                {"name", p->GetName()},
                {"price", p->GetPrice()},
                {"stock", p->GetStock()}
                });
        }

        res.set_content(result.dump(), "application/json");
        });

    /* -------- SEARCH PRODUCT -------- */
    svr.Get("/product/search", [&](const Request& req, Response& res) {
        addCORS(res);

        if (!req.has_param("id"))
        {
            res.set_content("{\"error\":\"Missing id\"}", "application/json");
            return;
        }

        int id = stoi(req.get_param_value("id"));
        if (!hashById.contains(id))
        {
            res.set_content("{\"error\":\"Product not found\"}", "application/json");
            return;
        }

        Product* p = tree.search(id);
        if (!p)
        {
            res.set_content("{\"error\":\"Product not found\"}", "application/json");
            return;
        }

        json out = {
            {"id", p->GetProductID()},
            {"name", p->GetName()},
            {"price", p->GetPrice()},
            {"stock", p->GetStock()}
        };

        res.set_content(out.dump(), "application/json");
        });

    svr.listen("0.0.0.0", 8080);
    return 0;
}
