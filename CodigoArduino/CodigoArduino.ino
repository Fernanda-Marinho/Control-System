#include <DHT_U.h>
#include <DHT.h>

/*
Autor: Professor Mercurie 
A TemperaturaAlvo é a temperatura que a câmara quente deveria manter. 
A margem na temperatura é uma variação admitida para que o relê não permaneça ligando e desligando com muita frequência.
Com a câmara aquecendo, para uma temperatura menor do que a Alvo + Margem o relê estará ligado. 
Para temperatura superior a Alvo - Margem o relê estará desligado enquanto a câmara perde temperatura.

Se a variável Aquecendo é igual a 1 a câmara quente estará aquecendo, se igual a 0 a câmara estará esfriando.
Se a variável Estado == 1 o programa estará rodando, caso contrário programa estará parado, com o relê desligado e não atualizando os sensores, 
evitando que caso o computador que lê os sinais reinicie o Arduino reinicie o programa acionando o relê
O programa só inicia após um comando passado pelo computador por meio da conexão USB e lida na variável s.
São admitidos 3 comandos passados por meio da variável s: "S" para iniciar o ensaio, "P" para parar e "I" para identificar os 
sensores DS18B20 que estão conectados ao sistema
*/

/* 
PINOS USADOS
 * 3    : Sensores DS18B20, protocolo ONEWIRE
 * 5    : Sensor DHT11 para medição da temperatura e umidade do ambiente externo
 * 7    : Relê para controle do sistema de aquecimento
*/
#include <Adafruit_Sensor.h>    // Usada para aqusição dos sinais do DHT11, temperatura e umidade do ambiente externo    
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>            // Protocolo usado pelos sensores DS18B20
#include <DallasTemperature.h>

#define PINO_DS18B20  3
#define PINO_DHT      5 
#define PINO_RELE     7
#define TIPO_DHT      DHT11
#define NDS18B20      8                  // Número de sensores usados no ensaio e que serão identificados 
                                         // a partir dos endereços nesse código. O ensaio não precisa usar
                                         // todos os endereços 
#define IntervaloLeituraSensores 2000    // Intervalo para leitura dos sensores (em ms)
#define IntervaloGravacaoResultados 20000 // intervalo para gravação dos valores lidos a partir dos sensores (em ms)
#define TemperaturaAlvo 45.0
#define MargemTemperatura 2.0

float TemperaturaCamara1, TemperaturaCamara2;
boolean Aquecendo;
boolean Estado;       
char s;

unsigned long int TLeituraSensores, TGravacaoResultados;

DHT_Unified dht1(PINO_DHT, TIPO_DHT);
float TempHumi[2];

OneWire oneWire(PINO_DS18B20);
DallasTemperature sensors(&oneWire);
// Endereço dos sensores DS18B20
uint8_t DS18B20 [17][8] = {{ 0x28, 0x07, 0x43, 0x79, 0xA2, 0x15, 0x03, 0x4F },
                           { 0x28, 0xAE, 0x4B, 0x79, 0xA2, 0x15, 0x03, 0xBB },
                           { 0x28, 0xC3, 0x5A, 0x79, 0xA2, 0x15, 0x03, 0x83 },
                           { 0x28, 0xEE, 0x88, 0x2E, 0x30, 0x16, 0x01, 0x72 },
                           { 0x28, 0x08, 0x8E, 0x79, 0xA2, 0x16, 0x03, 0xF0 },
                           { 0x28, 0xA6, 0x08, 0x79, 0xA2, 0x15, 0x03, 0xBD },
                           { 0x28, 0x9C, 0x1C, 0x79, 0xA2, 0x15, 0x03, 0xFC },
                           { 0x28, 0xEE, 0xFE, 0x48, 0x30, 0x16, 0x01, 0x1A },
                           { 0x28, 0xEE, 0x20, 0xBD, 0x2E, 0x16, 0x02, 0x4A },
                           { 0x28, 0xEE, 0x78, 0x4A, 0x30, 0x16, 0x01, 0x4A },
                           { 0x28, 0xEE, 0x31, 0x1C, 0x2F, 0x16, 0x02, 0x76 },
                           { 0x28, 0xEE, 0xFB, 0xC6, 0x2E, 0x16, 0x02, 0x58 },
                           { 0x28, 0xEE, 0xBF, 0x15, 0x30, 0x16, 0x01, 0x7B },
                           { 0x28, 0xEE, 0x4E, 0x4C, 0x30, 0x16, 0x01, 0x5B },
                           { 0x28, 0xEE, 0x22, 0x14, 0x30, 0x16, 0x01, 0xAF },
                           { 0x28, 0xEE, 0x2A, 0x15, 0x30, 0x16, 0x01, 0x1E },
                           { 0x28, 0xEE, 0x34, 0x14, 0x30, 0x16, 0x01, 0x4F }
                           };    
float TempDS18B20[NDS18B20]; // Variável para gravação das temperaturas dos sensores DS18B20


void setup()
{

    Serial.begin(9600);
    dht1.begin();
    
    // Pino do rele
    pinMode(PINO_RELE, OUTPUT);
    digitalWrite(PINO_RELE, LOW);
    
    // Inicializar os sensores DS18B20 e alterar a resolução para 12 bits
    sensors.begin();
    for (int i = 0; i < NDS18B20; i++)
    {
        sensors.setResolution(DS18B20[i], 12);
    }
    
    Aquecendo = 0;
    Estado = 1;

}

// Temperatura e umidade do ambiente
void TempUmiExternos(void)
{

    sensors_event_t event;
    
    dht1.temperature().getEvent(&event);
    if (isnan(event.temperature)) // Erro na leitura da temperatura
    {
        TempHumi[0] = 0.0;
    }
    else 
    {
        TempHumi[0] = event.temperature;
    }

    dht1.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) // Erro na leitura da umidade
    {
        TempHumi[1] = 0.0;
    }
    else 
    {
        TempHumi[1] = event.relative_humidity;
    }        
    
}


// Pega as temperaturas de todo os sensores DS18B20 ligados no barramento e armazena num vetor
void LerSensoresDS18B20(void)
{
    
    sensors.requestTemperatures();
    
    for (int i = 0; i < NDS18B20; i++){
        TempDS18B20[i] = LerSensorDS18B20(DS18B20[i]);
    }
    
}


// Dado o endereço do sensor ligado no barramento retorna a temperatura medida pelo mesmo
// Caso haja erro na leitura é retornado o valor 0.0
float LerSensorDS18B20(DeviceAddress deviceAddress)
{
    float tempC = sensors.getTempC(deviceAddress);
    
    if (tempC == -127.00) 
    {
        return 0.0;
    } 
    else 
    {
        return tempC;
    }
}


// Verifica o estado da câmara e aciona ou desliga o relê
// Se a câmara está aquecendo e com temperatura inferior à alvo + margem, o relê é acionado
// Se a câmara está perdendo temperatura e com a temperatura acima da alvo - margem, o relê é desligado
// Quando uma temperatura inferior à essa é atingida o relê é novamente acionado
void LigarDesligarRele(float TemperaturaCamara1)
{

    if (Aquecendo == 1)
    {
        if (TemperaturaCamara1 < (TemperaturaAlvo + MargemTemperatura))
        {
            Aquecendo = 1;    
        }
        else
        {
            Aquecendo = 0; 
        }
    }

    else
    {
        if (TemperaturaCamara1 > (TemperaturaAlvo - MargemTemperatura))
        {
            Aquecendo = 0;
        }
        else
        {
            Aquecendo = 1;
        }
    }

    // Aciona ou desliga o relê e o led para indicar
    if (Aquecendo == 1)
    {
        digitalWrite(PINO_RELE, HIGH);
    }
    else
    {
        digitalWrite(PINO_RELE, LOW);
    }
    
}


// Passa os resultados para o computador por meio da conexão serial
// Os dos primeiros valores são a umidade e temperatura externos
// Em seguida as temperaturas lidas pelo sensores DS18B20
// Os valores são separados por tabulação "\t"
void GravarResultados(float TempHumi[], float TempDS18B20[])
{

    Serial.print(TempHumi[0],6);
    Serial.print('\t');
    Serial.print(TempHumi[1],6);
    Serial.print('\t');
    
    for (int i = 0; i < NDS18B20; i++)
    {
        Serial.print(TempDS18B20[i],6);
        Serial.print('\t');
    }
    Serial.println(' ');
    
}
 
// Identifica todos os sensores DS18B20 ligados ao sistema
// É util para cadastrar o endereço de um novo sensor ou verificar se todos os sensores ligados ao sistema estão
// enviando sinal adequadamente
void IdentificarSensores(void)
{

    byte i;
    byte addr[8];

    Serial.print("Procurando dispositivos DS18B20\n\r");

    while(oneWire.search(addr)) 
    {
        Serial.print("\n\rEncontrado sensor \'DS18B20\' com endereco:\n\r");
        for( i = 0; i < 8; i++) 
        {
            Serial.print("0x");
            if (addr[i] < 16) 
            {
                Serial.print('0');

            }
            Serial.print(addr[i], HEX);

            if (i < 7) 
            {
                Serial.print(", ");
            }
        }

        if ( OneWire::crc8(addr, 7) != addr[7]) 
        {
            Serial.print("CRC nao e valido!\n");
            return;
        }
    }
    
    Serial.print("\n\r\n\rFinal da verificacao.\r\n");
    oneWire.reset_search();
    return;

}


void loop()
{
    
    // Verifica se foi mandado algum sinal do computador para iniciar o ensaio, parar o ensaio ou identificar os sensores
    if (Serial.available() > 0)
    {
        s = Serial.read();
    }
    
    switch (s)
    {
        case 'S': // Iniciar o ensaio
            Estado = 1;
            TLeituraSensores = millis() + IntervaloLeituraSensores;
            TGravacaoResultados = millis() + IntervaloGravacaoResultados;
            break;

        case 'P': // Parar o ensaio desligando o relê
            Estado = 0;
            Aquecendo = 0;   
            digitalWrite(PINO_RELE, LOW);
            break;
        
        case 'I': // Identificar os sensores DS18B20 ligados
            IdentificarSensores();
            break; 
    }
    
    // Se o ensaio está rodando
    if (Estado == 1)
    {
        
        // Caso tenha sido atingido o tempo para ler os sensores e atualizar as variáveis 
        if (millis() > TLeituraSensores)
        {
            TLeituraSensores = millis() + IntervaloLeituraSensores;
            
            // Ler as temperaturas dos sensores dentro das câmaras
            LerSensoresDS18B20(); 
            
            // Pegar a temperatura da Câmara 1 (Câmara quente) para controle do aquecedor
            // Os dois primeiros senosres DS18B20 sempre representarão a câmara quente
            TemperaturaCamara1 = (TempDS18B20[0] + TempDS18B20[1])/2;
            
            // Verificar se precisa acionar o relê
            if (TemperaturaCamara1 != 0.0)
            {
                LigarDesligarRele(TemperaturaCamara1);
            }

            // Temperatura e umidade do DHT11
            TempUmiExternos();

        }
        
        // Caso tenha sido atingido o tempo para mandar um novo conjunto de dados para o computador
        if (millis() > TGravacaoResultados)
        {
            TGravacaoResultados = millis() + IntervaloGravacaoResultados;    
            
            GravarResultados(TempHumi, TempDS18B20); 

        }

    }

}