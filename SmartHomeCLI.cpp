#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stack>
#include <map>
#include <sstream>
#include <functional>

// кольори в консолі (UX) 
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string CYAN    = "\033[36m";
    const std::string BOLD    = "\033[1m";
}


// абстаркції DIP - Dependency Inversion) Інтерфейс пристрою (ISP - Interface Segregation)
class IDevice {
public:
    virtual ~IDevice() = default;
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getStatus() const = 0;
};

// Інтерфейс команди
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};


// конкретні пристрої (LSP - Liskov Substitution)

class BaseDevice : public IDevice {
protected:
    std::string name;
    bool isOn;
public:
    BaseDevice(std::string n) : name(n), isOn(false) {}
    
    std::string getName() const override { return name; }
    
    std::string getStatus() const override {
        return name + "\t: " + (isOn ? Color::GREEN + "[ON]" + Color::RESET : Color::RED + "[OFF]" + Color::RESET);
    }
};

class Light : public BaseDevice {
public:
    Light(std::string n) : BaseDevice(n) {}
    void turnOn() override {
        isOn = true;
        std::cout << Color::YELLOW << ">>> Лампа (" << name << ") світить яскраво." << Color::RESET << "\n";
    }
    void turnOff() override {
        isOn = false;
        std::cout << ">>> Лампа (" << name << ") згасла.\n";
    }
};

class AirConditioner : public BaseDevice {
public:
    AirConditioner(std::string n) : BaseDevice(n) {}
    void turnOn() override {
        isOn = true;
        std::cout << Color::CYAN << ">>> Кондиціонер (" << name << ") охолоджує повітря." << Color::RESET << "\n";
    }
    void turnOff() override {
        isOn = false;
        std::cout << ">>> Кондиціонер (" << name << ") вимкнено.\n";
    }
};


// ФАБРИКА (OCP - Open/Closed Principle) Дозволяє створювати нові типи, не змінюючи логіку клієнта
class DeviceFactory {
public:
    static std::shared_ptr<IDevice> create(const std::string& type, const std::string& name) {
        if (type == "light") return std::make_shared<Light>(name);
        if (type == "ac") return std::make_shared<AirConditioner>(name);
        return nullptr;
    }
};

//(Command Pattern)


class TurnOnCommand : public ICommand {
    std::shared_ptr<IDevice> device;
public:
    TurnOnCommand(std::shared_ptr<IDevice> dev) : device(dev) {}
    void execute() override { device->turnOn(); }
    void undo() override { device->turnOff(); }
};

class TurnOffCommand : public ICommand {
    std::shared_ptr<IDevice> device;
public:
    TurnOffCommand(std::shared_ptr<IDevice> dev) : device(dev) {}
    void execute() override { device->turnOff(); }
    void undo() override { device->turnOn(); }
};


// (SRP - Single Responsibility) Відповідає ЛИШЕ за зберігання пристроїв
class DeviceManager {
    std::map<std::string, std::shared_ptr<IDevice>> devices;
public:
    void addDevice(std::shared_ptr<IDevice> dev) {
        if (dev) devices[dev->getName()] = dev;
    }

    std::shared_ptr<IDevice> getDevice(const std::string& name) {
        if (devices.find(name) != devices.end()) return devices[name];
        return nullptr;
    }

    void showAllStatus() const {
        std::cout << Color::BOLD << "\n--- СТАТУС БУДИНКУ ---" << Color::RESET << "\n";
        if (devices.empty()) std::cout << "(немає пристроїв)\n";
        for (const auto& pair : devices) {
            std::cout << pair.second->getStatus() << "\n";
        }
        std::cout << "----------------------\n";
    }
    
    bool exists(const std::string& name) const {
        return devices.find(name) != devices.end();
    }
};

// Відповідає ЛИШЕ за історію команд
class CommandInvoker {
    std::stack<std::unique_ptr<ICommand>> history;
public:
    void executeCommand(std::unique_ptr<ICommand> cmd) {
        cmd->execute();
        history.push(std::move(cmd));
    }

    void undo() {
        if (history.empty()) {
            std::cout << Color::YELLOW << "[INFO] Немає дій для скасування." << Color::RESET << "\n";
            return;
        }
        std::cout << Color::YELLOW << "[UNDO] Відміна останньої дії..." << Color::RESET << "\n";
        history.top()->undo();
        history.pop();
    }
};

//(User Interaction) Цей клас відповідає лише за спілкування з користувачем
class SmartHomeCLI {
    DeviceManager manager;
    CommandInvoker invoker;

public:
    void start() {
        std::string line, cmd, arg1, arg2;
        
        printWelcome();

        while (true) {
            std::cout << Color::BOLD << "> " << Color::RESET;
            std::getline(std::cin, line);
            if (line.empty()) continue;

            std::stringstream ss(line);
            ss >> cmd >> arg1 >> arg2;

            if (cmd == "exit") break;
            else if (cmd == "help") printHelp();
            else if (cmd == "status") manager.showAllStatus();
            else if (cmd == "undo") invoker.undo();
            else if (cmd == "add") handleAdd(arg1, arg2);
            else if (cmd == "on") handleOn(arg1);
            else if (cmd == "off") handleOff(arg1);
            else printError("Невідома команда. Напишіть 'help'.");
        }
    }

private:
    void handleAdd(const std::string& type, const std::string& name) {
        if (type.empty() || name.empty()) {
            printError("Формат: add [тип] [ім'я]");
            return;
        }
        if (manager.exists(name)) {
            printError("Пристрій з таким ім'ям вже існує!");
            return;
        }

        auto dev = DeviceFactory::create(type, name);
        if (dev) {
            manager.addDevice(dev);
            std::cout << Color::GREEN << "[OK] Пристрій '" << name << "' успішно додано." << Color::RESET << "\n";
        } else {
            printError("Невідомий тип пристрою. Доступні: light, ac");
        }
    }

    void handleOn(const std::string& name) {
        auto dev = manager.getDevice(name);
        if (dev) invoker.executeCommand(std::make_unique<TurnOnCommand>(dev));
        else printError("Пристрій не знайдено.");
    }

    void handleOff(const std::string& name) {
        auto dev = manager.getDevice(name);
        if (dev) invoker.executeCommand(std::make_unique<TurnOffCommand>(dev));
        else printError("Пристрій не знайдено.");
    }

    void printError(const std::string& msg) {
        std::cout << Color::RED << "[ПОМИЛКА] " << msg << Color::RESET << "\n";
    }

    void printWelcome() {
        std::cout << Color::BOLD << "=== SMART HOME CONTROL SYSTEM v2.0 (SOLID) ===" << Color::RESET << "\n";
        std::cout << "Введіть 'help' для списку команд.\n";
    }

    void printHelp() {
        std::cout << "\nДоступні команди:\n";
        std::cout << "  add [light|ac] [name]  - Додати новий пристрій\n";
        std::cout << "  on [name]              - Увімкнути пристрій\n";
        std::cout << "  off [name]             - Вимкнути пристрій\n";
        std::cout << "  undo                   - Скасувати останню дію\n";
        std::cout << "  status                 - Показати статус усіх пристроїв\n";
        std::cout << "  exit                   - Вийти\n\n";
    }
};

int main() {
    SmartHomeCLI app;
    app.start();Smart Home CLI
    return 0;
}