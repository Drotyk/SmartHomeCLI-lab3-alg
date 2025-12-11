Smart Home CLIКонсольна система керування "Розумним будинком" на C++17. Проект демонструє практичне застосування патернів проектування та принципів SOLID.
Технології та АрхітектураМова: C++17 (STL, Smart Pointers)
Патерни:Command: Інкапсуляція дій для реалізації undo.Factory Method: Гнучке створення нових пристроїв.Dependency Injection: Робота через абстракції (IDevice).SOLID: Дотримання принципів SRP, OCP, LSP, DIP для чистоти коду.
Компіляція та ЗапускBashg++ -std=c++17 main.cpp -o smarthome
./smarthome
Список команд
Команда   Аргументи        Опис
add       [type] [name]    Створити пристрій (light або ac)
on        [name]           Увімкнути пристрій
off       [name]           Вимкнути пристрійundo-Скасувати останню діюstatus-Стан усіх пристроївexit-Вихід з програмиПриклад використання:Plaintext> add light Kitchen
> on Kitchen
> undo
