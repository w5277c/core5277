# core5277
Операционная система для микроконтроллеров Atmel серии ATmega и частично для ATtiny, разработана на ассемблере(AVRA).
На данный момент разрабатывается с поддержкой ATtiny85,ATmega8/88/16/168/328,AT90CAN32
Также в проект включены ATtiny13A,ATtiny45 и прочие в будущем, но с сильно ограниченным функционалом, так как для среднестатистического функционала необходимо минимум 8КБ FLASH.

Благодаря унифицированной прослойки в виде процедур ядра и драйверов достигается независимость кода от конкретного железа(по большему счету, что в свою очередь приводит к дополнительному потреблению ресурсов МК) и заметно уменьшает объем кода верхнего уровня.
Автор лично переносил свои проекты в 7-12КБ FLASH между чипами ATmega8,88,16,328 буквально изменяя только несколько строк кода.

Ассемблер выбран не ради максимальной экономии ресурсов(хотя это свойство в проекте крайне полезно), а попросту по причине эстетического удовольствия.

Проект предлагает легкость разработки прошивок благодаря наработанному функционалу операционной системы.
Позволяет абстрагироваться от ненужных высокоуровневых прослоек для МК типа Си и влияния компиляторов.

Данный проект автор использует в своих наработках - прошивки конечного железа и шлюзов для автоматизации(проект http://5277.ru)

Операционная система может быть запущена в нескольких режимах:
1) TS_MODE_NO - простое переключение, механизмы ядра для переключения задач отсутствуют
2) TS_MODE_EVENT - кооперативная
3) TS_MODE_TIME - вытесняющая многозадачность

Весь проект построен на подгрузке только задействованных библиотек, что сильно влияет на размер итоговой прошивки, в том числе и режимы ОС.

Поддержка программных таймеров типа A, высокочастотного таймера отсчета пауз B и отдельного быстрого таймера C, что позволяет обеспечить независимость от конкретного железа.
Несколько реализаций UART, аппаратная, программная, программная быстрая, и два типа логирования.
Реализация BEEP'ера, датчиков температуры и влажности, SD карты и прочее.
Механизмы ввода/вывода c буферизацией на базе UART 230400 8N1(для 16Mhz)
Наличие различных процедур математики, конвертирования, работы со строками и прочее.
Планы на поддержку символьных и LED дисплеев с учетом унифицированной графической прослойки, своя легковесная файловая система, эмулятор, поддержка контроллеров VGA и MIDI/MP3/lossless и прочее.
Проект находится в ранней стадии разработки, но уже позволяет автору создавать сложные по функционалу прошивки для своих устройств.

Похожий проект также начат для STM32





