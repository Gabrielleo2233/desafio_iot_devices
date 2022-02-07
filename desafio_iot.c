
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <devicetree.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>

/* 0,5 mseg = 500 useg */
#define SLEEP_TIME   0.5

#define LED0_NODE DT_ALIAS(led0) //LED0_NODE é o mesmo que DT_ALIAS(led0) -> DT_ALIAS(led0) identifica o nó da devicetree (ID do nó)
#define LED1_NODE DT_ALIAS(led1) //Led 1
#define SW0_NODE DT_ALIAS(sw0) //Botão 0
#define SW1_NODE DT_ALIAS(sw1) //Botão 1

#if (DT_NODE_HAS_STATUS(LED0_NODE, okay) && DT_NODE_HAS_STATUS(LED1_NODE, okay)) //Verifica se o houve sucesso na identificação do nó
//Atribui Label, PIN e Flags aos Led's
//LED0
#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios) 
#define PINL0 DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGSL0	DT_GPIO_FLAGS(LED0_NODE, gpios)
//LED1
#define LED1 DT_GPIO_LABEL(LED1_NODE, gpios)
#define PINL1 DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGSL1	DT_GPIO_FLAGS(LED1_NODE, gpios)

#else //Falha na identificação do nó

#error "Unsupported board: led0 or led1 devicetree alias is not defined"
#define LED0 ""
#define PINL0 0
#define FLAGSL0	0

#define LED1 ""
#define PINL1 0
#define FLAGSL1 0
#endif

#if (!DT_NODE_HAS_STATUS(SW0_NODE, okay) || !DT_NODE_HAS_STATUS(SW1_NODE, okay)) //Verifica se a identificação dos nós para botão 1 e botão 2 teve sucesso
#error "Unsupported board: sw0 or sw1 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,{0});
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios,{0});
static struct gpio_callback button0_cb_data;
static struct device *devL0; //Objeto devL0 aponta para classe device

void button0_pressed(const struct device *devB0, struct gpio_callback *cb, uint32_t pins) //Função chamada quando o botão 0 é pressionado
{
	gpio_pin_set(devL0, PINL0, 1); //Leva HIGH para a gpio do LED 0
    //AQUI DEVE-SE LER O VALOR DO COUNT REGISTER
    k_msleep(SLEEP_TIME); //tempo de 500us
    gpio_pin_set(devL0, PINL0, 0)
}

void main(void)
{
	const struct device *devL1; 
    int retL0;
    int retL1;
    int retB0;
    int retB1;
   
    //BOTÕES
    if (!device_is_ready(button0.port)) { //Verificando se a porta do botão 0 está pronta
		printk("Error: button device %s is not ready\n",
		       button0.port->name);
		return;
	}
    if (!device_is_ready(button1.port)) { //Verificando se a porta do botão 1 está pronta
		printk("Error: button device %s is not ready\n",
		       button1.port->name);
		return;
    }

    retB0 = gpio_pin_configure_dt(&button0, GPIO_INPUT); //gpio_pin_configure é uma função da SDK que configura GPIO_INPUT
	retB1 = gpio_pin_configure_dt(&button1, GPIO_INPUT);
    
    //casos de falha
    if (retB0 != 0) { 
		printk("Error %d: failed to configure %s pin %d\n", retB0, button0.port->name, button0.pin);
		return;
	}
    else if (retB1 != 0) {
        printk("Error %d: failed to configure %s pin %d\n", retB1, button1.port->name, button1.pin);
		return;
    }
    gpio_init_callback(&button0_cb_data, button0_pressed, BIT(button0.pin)); //Escuta a função button0_pressed
    gpio_add_callback(button0.port, &button0_cb_data);
	printk("Set up button0 at %s pin %d\n", button0.port->name, button0.pin);

    //LEDS

    devL0 = device_get_binding(LED0); //Busca informações de LED0 no SDK e joga na variável devL0
    devL1 = device_get_binding(LED1); 

	if ((devL0 == NULL) || (devL1 == NULL)){
		return; //Falha na busca de informações no SDK
	}

    retL0 = gpio_pin_configure(devL0, PINL0, GPIO_OUTPUT_ACTIVE | FLAGSL0);
    retL1 = gpio_pin_configure(devL1, PINL1, GPIO_OUTPUT_ACTIVE | FLAGSL1);

    if ((retL0 < 0) || (retL1 < 0)) {
		return; //Falha na configuração dos pinos dos LED's
	}

    while (1){ //loop infinito
        int valB1 = gpio_pin_get_dt(&button1); //Lê se o botão está pressionado ou não

        gpio_pin_set(devL1, PINL1, valB1); // O segundo LED acende para Botão 1 pressionado e apagada para Botão 1 solto

    }
}