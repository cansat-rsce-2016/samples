#include <stdio.h>
#include <libserialport.h>

/*
 * Пример работы с библиотекой lib-serial-port
 * Подразумевет установленную библиотеку libserialport
 */

int main()
{
	// перебираем все порты которые есть в системе
	struct sp_port ** ports;
	sp_list_ports(&ports);
	int i = 0;
	while (ports[i] != NULL)
	{
		printf("port name: %s\n", sp_get_port_name(ports[i]));
		i++;
	}

	// открываем первый попавшийся порт
	struct sp_port * port = ports[0];
	if(!port){
		printf("Нет ни одного порта в системе\n");
		return 1;
	}
	if (SP_OK != sp_open(port, SP_MODE_READ | SP_MODE_WRITE))
	{
		printf("Не могу открыть порт!\n");
		return 2;
	}
	printf("Успешно открыл порт %s\n", sp_get_port_name(port));

	// настраиваем этот порт
	sp_set_baudrate(port, 9600);
	sp_set_bits(port, 8);
	sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);
	sp_set_parity(port, SP_PARITY_NONE);
	sp_set_stopbits(port, 1);

	// пишем данные
	char out_buffer[] = "Hello world!";
	sp_blocking_write(port, out_buffer, sizeof(out_buffer), 0);

	// читаем данные
	char in_buffer[sizeof(out_buffer)];
	sp_blocking_read(port, in_buffer, sizeof(in_buffer), 0);

	printf("Прочитано из порта: \"%s\"\n", out_buffer);

	// готово!
	return 0;
}
