# eeprom_emul
EEPROM emulator for STM32F100 and 20's 32 bit variables in 3 page of N25Q032A SPI serial flash memory

##Задание:

Нужно разработать модуль на языке C. 
Предложить решение и реализовать основную логику модуля
 
Задача: имеется 20 переменных размером 32 бита, которые должны быть доступны для быстрого доступа при работе устройства. Разные переменные изменяются с разной частотой, но среднее количество изменений составляет 3 раза в мин.
Данные должны восстанавливаться после перезагрузки и потеря данных является критической. При разработке необходимо исходить из того, что питание прибора может пропадать в любой момент.
В качестве энергонезависимой памяти используется флешка, например, N25Q032A с размером сектора 4кбайта и количеством стираний 10 тыс.
Под настройки отводится 3 сектора (размером 4кбайта)
Необходимо разработать концепцию модуля хранения и реализовать логику модуля. 
Для работы с флеш памятью предоставлен модуль, который обеспечивает запись на флеш. Интерфейс модуля представлен в файле во вложении.
 
Модуль будет выполняться на микроконтроллере, например, STM32, хотя желательно, чтобы он мог выполняться и на PIC, AVR произвольной разрядности.
Операционная система не используется.
Функции записи в буфер могут вызываться из различных прерываний и фоновых потоков программы. 
Запись должна выполняться максимально быстро.
Место в ОЗУ должно быть использовано максимально эффективно.

##Лицензия: 

Только для ознокомления, любое использование в комерческих продуктах третьих лиц или фирм недопустимо.
Все права принадлежат Ситникову Е.Н.
