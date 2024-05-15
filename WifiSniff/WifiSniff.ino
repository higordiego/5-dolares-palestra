#include "freertos/FreeRTOS.h" // Biblioteca FreeRTOS
#include "esp_wifi.h" // Biblioteca ESP32 Wi-Fi
#include "esp_wifi_types.h" // Tipos de dados relacionados ao Wi-Fi do ESP32
#include "esp_system.h" // Funções do sistema do ESP32
#include "esp_event.h" // Eventos do ESP32
#include "esp_event_loop.h" // Loop de eventos do ESP32
#include "nvs_flash.h" // Armazenamento não volátil (NVS) do ESP32
#include "driver/gpio.h" // Manipulação de pinos GPIO no ESP32

#define LED_GPIO_PIN 5 // Pino GPIO para controle do LED
#define WIFI_CHANNEL_SWITCH_INTERVAL 500 // Intervalo de troca de canal Wi-Fi em milissegundos
#define WIFI_CHANNEL_MAX 13 // Número máximo de canais Wi-Fi
#define PACKET_DELAY 100 // Atraso entre manipulações de pacotes em milissegundos

uint8_t level = 0, channel = 1;

// Estrutura que define o país para configuração de canais Wi-Fi
static wifi_country_t wifi_country = {.cc="CN", .schan = 1, .nchan = 13}; 

// Estrutura para o cabeçalho MAC IEEE 802.11
typedef struct {
  unsigned frame_ctrl:16;
  unsigned duration_id:16;
  uint8_t addr1[6]; /* endereço do receptor */
  uint8_t addr2[6]; /* endereço do remetente */
  uint8_t addr3[6]; /* endereço de filtragem */
  unsigned sequence_ctrl:16;
  uint8_t addr4[6]; /* opcional */
} wifi_ieee80211_mac_hdr_t;

// Estrutura para um pacote IEEE 802.11 Wi-Fi
typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* dados da rede terminados com 4 bytes de verificação (CRC32) */
} wifi_ieee80211_packet_t;

// Manipulador de eventos do sistema do ESP32
static esp_err_t event_handler(void *ctx, system_event_t *event) {
  return ESP_OK;
}

// Inicializa o sniffer Wi-Fi
void wifi_sniffer_init(void) {
  nvs_flash_init(); // Inicializa o armazenamento não volátil (NVS)
  tcpip_adapter_init(); // Inicializa o adaptador TCP/IP
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) ); // Inicializa o loop de eventos do ESP32
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // Configurações padrão do Wi-Fi
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) ); // Inicializa o Wi-Fi
  ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); // Configura o país para o alcance do canal [1, 13]
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) ); // Configura o armazenamento do Wi-Fi
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) ); // Configura o modo do Wi-Fi
  ESP_ERROR_CHECK( esp_wifi_start() ); // Inicia o Wi-Fi
  esp_wifi_set_promiscuous(true); // Configura o modo promíscuo
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler); // Define o callback para manipulação de pacotes promíscuos
}

// Configura o canal Wi-Fi para o sniffer
void wifi_sniffer_set_channel(uint8_t channel) {
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE); // Configura o canal Wi-Fi
}

// Converte o tipo de pacote Wi-Fi para uma string
const char * wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type) {
  switch(type) {
    case WIFI_PKT_MGMT: return "Gestão";
    case WIFI_PKT_DATA: return "Dados";
    default:  
    case WIFI_PKT_MISC: return "Outro";
  }
}

// Manipulador de pacotes Wi-Fi capturados
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  // Exibe detalhes do pacote capturado
  printf("TIPO DE PACOTE=%s, CANAL=%02d, RSSI=%02d,"
    " ENDEREÇO DO RECEPTOR=%02x:%02x:%02x:%02x:%02x:%02x,"
    " ENDEREÇO DO REMETENTE=%02x:%02x:%02x:%02x:%02x:%02x,"
    " ENDEREÇO DE FILTRAGEM=%02x:%02x:%02x:%02x:%02x:%02x\n",
    wifi_sniffer_packet_type2str(type),
    ppkt->rx_ctrl.channel,
    ppkt->rx_ctrl.rssi,
    /* ENDEREÇO DO RECEPTOR */
    hdr->addr1[0],hdr->addr1[1],hdr->addr1[2],
    hdr->addr1[3],hdr->addr1[4],hdr->addr1[5],
    /* ENDEREÇO DO REMETENTE */
    hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
    hdr->addr2[3],hdr->addr2[4],hdr->addr2[5],
    /* ENDEREÇO DE FILTRAGEM */
    hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
    hdr->addr3[3],hdr->addr3[4],hdr->addr3[5]
  );

  delay(PACKET_DELAY); // Adiciona um atraso entre manipulações de pacotes
}

// Configurações iniciais do programa
void setup() {
  Serial.begin(19200); // Inicializa a comunicação serial
  delay(10);
  wifi_sniffer_init(); // Inicializa o sniffer Wi-Fi
  pinMode(LED_GPIO_PIN, OUTPUT); // Configura o pino do LED como saída
}

// Loop principal do programa
void loop() {
  delay(1000); // Aguarda por um segundo
  
  // Inverte o estado do LED
  if (digitalRead(LED_GPIO_PIN) == LOW)
    digitalWrite(LED_GPIO_PIN, HIGH);
  else
    digitalWrite(LED_GPIO_PIN, LOW);

  vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS); // Aguarda o intervalo antes de trocar de canal
  wifi_sniffer_set_channel(channel); // Define o canal do sniffer Wi-Fi
  channel = (channel % WIFI_CHANNEL_MAX) + 1; // Incrementa o número do canal, retornando a 1 se exceder o limite máximo
}