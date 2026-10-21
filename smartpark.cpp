#include <array>
#include <cctype>
#include <algorithm>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <emscripten/bind.h>

using namespace std;
using namespace emscripten;

class SmartPark {
private:
    static constexpr int SLOT_COUNT = 12;

    struct Slot {
        bool occupied = false;
        string vehicle;
    };

    // 1) ARRAY: fixed number of parking slots
    array<Slot, SLOT_COUNT> slots{};

    // 2) QUEUE: vehicles waiting for a free slot (FIFO)
    queue<string> waitingQueue;

    // 3) STACK: recently exited vehicles (LIFO)
    stack<string> exitedStack;

    // 4) LINKED LIST: parking activity history
    struct HistoryNode {
        string text;
        HistoryNode* next;
        explicit HistoryNode(const string& value) : text(value), next(nullptr) {}
    };
    HistoryNode* historyHead = nullptr;
    HistoryNode* historyTail = nullptr;

    // 5) HASH TABLE: active vehicle registration -> slot index
    unordered_map<string, int> vehicleToSlot;

    static string normalize(const string& input) {
        string result;
        for (unsigned char ch : input) {
            if (!isspace(ch)) result += static_cast<char>(toupper(ch));
        }
        return result;
    }

    void addHistory(const string& text) {
        HistoryNode* node = new HistoryNode(text);
        if (historyHead == nullptr) {
            historyHead = historyTail = node;
        } else {
            historyTail->next = node;
            historyTail = node;
        }
    }

    bool isWaiting(const string& vehicle) const {
        queue<string> copy = waitingQueue;
        while (!copy.empty()) {
            if (copy.front() == vehicle) return true;
            copy.pop();
        }
        return false;
    }

    int findFreeSlot() const {
        for (int i = 0; i < SLOT_COUNT; ++i) {
            if (!slots[i].occupied) return i;
        }
        return -1;
    }

    void assignVehicleToSlot(const string& vehicle, int index) {
        slots[index].occupied = true;
        slots[index].vehicle = vehicle;
        vehicleToSlot[vehicle] = index;
        addHistory(vehicle + " parked in slot P" + to_string(index + 1));
    }

public:
    SmartPark() = default;

    // Avoid copying a raw-pointer linked list.
    SmartPark(const SmartPark&) = delete;
    SmartPark& operator=(const SmartPark&) = delete;

    ~SmartPark() {
        HistoryNode* current = historyHead;
        while (current != nullptr) {
            HistoryNode* next = current->next;
            delete current;
            current = next;
        }
    }

    vector<int> getSlots() const {
        vector<int> result;
        result.reserve(SLOT_COUNT);
        for (const Slot& slot : slots) result.push_back(slot.occupied ? 1 : 0);
        return result;
    }

    string getSlotVehicle(int slotNumber) const {
        if (slotNumber < 1 || slotNumber > SLOT_COUNT) return "";
        const Slot& slot = slots[slotNumber - 1];
        return slot.occupied ? slot.vehicle : "";
    }

    int availableCount() const {
        int count = 0;
        for (const Slot& slot : slots) if (!slot.occupied) ++count;
        return count;
    }

    int occupiedCount() const { return SLOT_COUNT - availableCount(); }
    int waitingCount() const { return static_cast<int>(waitingQueue.size()); }
    int totalSlots() const { return SLOT_COUNT; }

    string parkVehicle(const string& rawInput) {
        string vehicle = normalize(rawInput);
        if (vehicle.empty()) return "Please enter a vehicle registration number.";
        if (vehicleToSlot.find(vehicle) != vehicleToSlot.end())
            return vehicle + " is already parked.";
        if (isWaiting(vehicle))
            return vehicle + " is already in the waiting queue.";

        int freeIndex = findFreeSlot();
        if (freeIndex == -1) {
            waitingQueue.push(vehicle);
            addHistory(vehicle + " joined the waiting queue");
            return "Parking is full. " + vehicle + " was added to the waiting queue.";
        }

        assignVehicleToSlot(vehicle, freeIndex);
        return vehicle + " parked successfully in P" + to_string(freeIndex + 1) + ".";
    }

    string exitVehicle(const string& rawInput) {
        string vehicle = normalize(rawInput);
        if (vehicle.empty()) return "Please enter a vehicle registration number.";

        auto found = vehicleToSlot.find(vehicle);
        if (found == vehicleToSlot.end())
            return vehicle + " is not currently parked.";

        int index = found->second;
        slots[index].occupied = false;
        slots[index].vehicle.clear();
        vehicleToSlot.erase(found);
        exitedStack.push(vehicle);
        addHistory(vehicle + " exited from P" + to_string(index + 1));

        string message = vehicle + " exited from P" + to_string(index + 1) + ".";

        // The first waiting vehicle receives the newly freed slot.
        if (!waitingQueue.empty()) {
            string nextVehicle = waitingQueue.front();
            waitingQueue.pop();
            assignVehicleToSlot(nextVehicle, index);
            message += " " + nextVehicle + " moved from the waiting queue into P"
                       + to_string(index + 1) + ".";
        }
        return message;
    }

    string searchVehicle(const string& rawInput) const {
        string vehicle = normalize(rawInput);
        if (vehicle.empty()) return "Enter a vehicle registration number.";

        auto found = vehicleToSlot.find(vehicle);
        if (found != vehicleToSlot.end())
            return vehicle + " is parked in slot P" + to_string(found->second + 1) + ".";

        if (isWaiting(vehicle))
            return vehicle + " is in the waiting queue.";

        return vehicle + " was not found.";
    }

    string getWaitingQueue() const {
        if (waitingQueue.empty()) return "No waiting vehicles.";
        queue<string> copy = waitingQueue;
        string result;
        while (!copy.empty()) {
            if (!result.empty()) result += "\n";
            result += copy.front();
            copy.pop();
        }
        return result;
    }

    string getExitedStack() const {
        if (exitedStack.empty()) return "No exited vehicles yet.";
        stack<string> copy = exitedStack;
        string result;
        while (!copy.empty()) {
            if (!result.empty()) result += "\n";
            result += copy.top();
            copy.pop();
        }
        return result;
    }

    string getHistory() const {
        if (historyHead == nullptr) return "No parking history yet.";
        string result;
        for (HistoryNode* node = historyHead; node != nullptr; node = node->next) {
            if (!result.empty()) result += "\n";
            result += node->text;
        }
        return result;
    }
};

EMSCRIPTEN_BINDINGS(smartpark_module) {
    register_vector<int>("IntVector");
    class_<SmartPark>("SmartPark")
        .constructor<>()
        .function("getSlots", &SmartPark::getSlots)
        .function("getSlotVehicle", &SmartPark::getSlotVehicle)
        .function("availableCount", &SmartPark::availableCount)
        .function("occupiedCount", &SmartPark::occupiedCount)
        .function("waitingCount", &SmartPark::waitingCount)
        .function("totalSlots", &SmartPark::totalSlots)
        .function("parkVehicle", &SmartPark::parkVehicle)
        .function("exitVehicle", &SmartPark::exitVehicle)
        .function("searchVehicle", &SmartPark::searchVehicle)
        .function("getWaitingQueue", &SmartPark::getWaitingQueue)
        .function("getExitedStack", &SmartPark::getExitedStack)
        .function("getHistory", &SmartPark::getHistory);
}
