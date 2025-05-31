#include <iostream>
#include <vector>
#include <string>
#include <cstdlib> 
#include <ctime> // For time()
#include <iomanip> 
namespace StockSim { // Namespace to encapsulate stock simulation classes
using namespace std;

class Stock { // Base class for all stocks(abstract)
protected:  
    int id; // Unique identifier for the stock
    string name; // Name of the stock
    double currentPrice; // Current price of the stock
    string riskLevel; // Risk level of the stock (Low, Medium, High)

public:
    Stock(int id, string name, double price, string risk) // Constructor to initialize stock attributes
        : id(id), name(name), currentPrice(price), riskLevel(risk) {} 

    virtual ~Stock() {} // Virtual destructor for proper cleanup of derived classes

    virtual void updatePrice() = 0; // Pure virtual function to update stock price, must be implemented by derived classes

    string getName() const { return name; } // Getter for stock name
    double getPrice() const { return currentPrice; } // Getter for current price
    string getRiskLevel() const { return riskLevel; } // Getter for risk level
    int getId() const { return id; } // Getter for stock ID

   
    void setPrice(double price) { currentPrice = price; } // Setter for current price

    virtual void display() const { // Display stock information
        cout << setw(2) << id << ". " << setw(12) << name  // Display stock name
             << " | $" << setw(8) << fixed << setprecision(2) << currentPrice  // Display current price
             << " | Risk: " << riskLevel; // Display risk level
    }

    friend ostream& operator<<(ostream& os, const Stock& stock) { // Overloaded operator to print stock information
        os << stock.name << " ($" << stock.currentPrice << ", " << stock.riskLevel << ")"; // Output format
        return os;
    }
};

class SimulatedStock : public Stock { // Derived class for simulated stocks
protected:
    vector<double> priceHistory; // Vector to store price history for the stock

public:
    SimulatedStock(int id, std::string name, double price, std::string risk) // Constructor to initialize simulated stock attributes
        : Stock(id, name, price, risk) { // Call base class constructor
        priceHistory.push_back(price); // Initialize price history with the current price
    }

    void updatePrice() override { // Override to update stock price based on risk level
        double volatility = (riskLevel == "High") ? 0.2 : (riskLevel == "Medium") ? 0.1 : 0.05; // Set volatility based on risk level
        double change = ((rand() % 201) - 100) / 100.0 * volatility * currentPrice; // Calculate price change based on volatility
        currentPrice += change; // Update current price with the calculated change
        if (currentPrice < 1) currentPrice = 1; // Ensure price does not go below 1
        priceHistory.push_back(currentPrice); // Add new price to history
    }

    const vector<double>& getHistory() const { return priceHistory; } // Getter for price history

    void display() const override { // Override to display stock information along with price history
        Stock::display(); // Call base class display method
        cout << " | Day " << priceHistory.size(); // Display the current day based on price history size
    }
};

class UserOwnedStock : public SimulatedStock { // Derived class for stocks owned by the user
private:
    int quantity;

public:
    UserOwnedStock(SimulatedStock* base, int qty) // Constructor to initialize user-owned stock attributes
        : SimulatedStock(base->getId(), base->getName(), base->getPrice(), base->getRiskLevel()), quantity(qty) {}

    int getQuantity() const { return quantity; } // Getter for quantity of stocks owned

    double getTotalValue() const { return quantity * currentPrice; } // Calculate total value of owned stocks

    void buy(int qty) { quantity += qty; } // Buy more stocks, increasing the quantity owned
    void sell(int qty) { // Sell stocks, decreasing the quantity owned
        if (qty <= quantity) quantity -= qty;
        else cout << "Not enough stock to sell.\n";
    }

    void display() const override { // Override to display user-owned stock information
        SimulatedStock::display();
        cout << " | Quantity: " << quantity << " | Value: $" << fixed << setprecision(2) << getTotalValue() << "\n"; // Display quantity and total value
    }
};

class UserPortfolio { // Class representing the user's portfolio
private:
    double balance;
    vector<UserOwnedStock*> ownedStocks;    // Vector to store stocks owned by the user

public:
    UserPortfolio(double initialBalance = 3000.0) : balance(initialBalance) {} // Constructor to initialize portfolio with an initial balance

    ~UserPortfolio() { // Destructor to clean up dynamically allocated memory
        for (auto stock : ownedStocks)
            delete stock;
    }

    void display() const {
        cout << "\n~ This is Your Portfolio ~\n";
        cout << "Balance: $" << fixed << setprecision(2) << balance << "\n"; // Display current balance
        if (ownedStocks.empty()) { // Check if there are no stocks owned
            cout << "No stocks owned yet\n";
        } else {
            for (const auto& stock : ownedStocks) // Loop through owned stocks and display each one
                stock->display();
        }
    }

    void buyStock(SimulatedStock* s, int qty) { // Function to buy stocks
        double total = s->getPrice() * qty;  // Calculate total cost of stocks to be bought
        if (total > balance) { // Check if the user has enough balance
            cout << "Insufficient balance.\n"; 
            return;
        }
        balance -= total;

        for (auto& stock : ownedStocks) { // Check if the stock is already owned
            if (stock->getName() == s->getName()) {
                stock->buy(qty); // If already owned, increase the quantity
                return;
            }
        }
        ownedStocks.push_back(new UserOwnedStock(s, qty)); // If not owned, create a new UserOwnedStock and add it to the portfolio
    }

    void sellStock(string stockName, int qty) { // Function to sell stocks
        for (size_t i = 0; i < ownedStocks.size(); ++i) { // Loop through owned stocks to find the stock to sell
            if (ownedStocks[i]->getName() == stockName) {
                if (ownedStocks[i]->getQuantity() >= qty) { // Check if the user has enough quantity to sell
                    double income = qty * ownedStocks[i]->getPrice(); // Calculate income from selling stocks
                    ownedStocks[i]->sell(qty); // Decrease the quantity of stocks owned
                    balance += income;
                    if (ownedStocks[i]->getQuantity() == 0) { // If quantity becomes zero, remove the stock from the portfolio
                        delete ownedStocks[i];
                        ownedStocks.erase(ownedStocks.begin() + i);   
                    }
                    return;
                } else {
                    cout << "Not enough quantity.\n";
                    return;
                }
            }
        }
        cout << "Stock not found in portfolio.\n";
    }

    void updatePrices() { // Function to update prices of all owned stocks
        for (auto& stock : ownedStocks)
            stock->updatePrice();
    }

    double getBalance() const { return balance; } // Getter for current balance
};

} // namespace StockSim


using namespace StockSim; // Use the StockSim namespace to access the stock simulation classes
using namespace std;



int main() {
    srand(time(0)); // Seed the random number generator for price updates

    vector<SimulatedStock*> market = { // Initialize the market with simulated stocks
        new SimulatedStock(1, "Apple", 211.0, "Medium"),
        new SimulatedStock(2, "Google", 165.0, "Medium"),
        new SimulatedStock(3, "Amazon", 205.0, "High"),
        new SimulatedStock(4, "McDonald's", 314.0, "Low"),
        new SimulatedStock(5, "UnitedHealth", 60.0, "Low"),
        new SimulatedStock(6, "Tesla", 342.0, "High"),
        new SimulatedStock(7, "NVDA", 134.0, "High"),
        new SimulatedStock(8, "Microsoft", 453.0, "Medium"),
        new SimulatedStock(9, "META", 643.0, "High")
    };

    UserPortfolio user; // Create a user portfolio with an initial balance
    int choice;
    do{
        cout << "\n~ This is investment simulator ~\n";
        cout << "1. Show market\n";
        cout << "2. Buy stock\n";
        cout << "3. Sell stock\n";
        cout << "4. Show portfolio\n";
        cout << "5. Simulate next day\n";   
        cout << "0. Exit\n";
        cout << "Please, choose an action(number): ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "\n~ Market Stocks ~\n";
                for (auto& s : market) { // Loop through the market and display each stock
                    s->display();
                    cout << "\n";
                }
                break;
            case 2: {
                int id, qty; 
                cout << "Enter stock ID to buy: \n";
                for (auto& s : market) { // Display available stocks with their IDs
                    s->display();
                    cout << "\n";
                }
                cin >> id;
                cout << "Enter quantity: ";
                cin >> qty;
                if (id >= 1 && id <= market.size()) { // Check if the entered ID is valid
                    user.buyStock(market[id - 1], qty); // Buy the stock with the specified ID and quantity
                } else {
                    cout << "Invalid ID.\n";
                }
                break;
            }
            case 3: {
                string name;
                int qty;
                cout << "Enter stock name to sell: ";
                cin >> ws; // Clear any leading whitespace
                getline(cin, name); // Read the stock name including spaces
                cout << "Enter quantity: ";
                cin >> qty;
                user.sellStock(name, qty);
                break;
            }
            case 4:
                user.display(); // Display the user's portfolio
                break;
            case 5:
                cout << "Simulating next day...\n";

                for (auto& s : market) s->updatePrice(); // Update prices of all stocks in the market
                user.updatePrices(); // Update prices of all stocks owned by the user
                cout << "Changes simulated! Here's your updated portfolio:\n";
                user.display(); // Display the updated portfolio after simulating the next day
                break;
            case 0:
                cout << "Goodbye! Please return later!\n";
                break;
            default:
                cout << "Invalid option.\n";
        }
    } while (choice != 0);

    for (auto s : market) // Clean up dynamically allocated memory for market stocks
        delete s;

    return 0;
}
