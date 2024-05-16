# Mini Robô Conectado à IoT para Tarefas Domésticas
TIA MACKENZIE: 20501994

## Descrição
Este repositório contém o desenvolvimento de um mini robô conectado à Internet das Coisas (IoT), projetado para auxiliar na realização de tarefas domésticas, como pegar objetos. O projeto integra várias plataformas de desenvolvimento, sensores, atuadores e impressões 3D para criar um robô versátil e inteligente. A aplicação deste robô pode melhorar o bem-estar e a qualidade de vida dos usuários, especialmente aqueles com dificuldades de mobilidade.

## Vídeo-demonstração do funcionamento com MQTT
<a href="https://www.youtube.com/watch?v=yo9W-R0bA5Q&t=499s" target="_blank">Vídeo do unboxing no meu canal do YouTube, detalhando como foi a construção do robô, os desafios e a configuração, passo a passo com o funcionamento</a> 

<a href="https://youtu.be/j-XhmQF3l8c" target="_blank">Vídeo privado no qual disponibilizei mostrando um pouco mais a construção do robô</a> 



## Funcionalidades
- **Controle Remoto via Wi-Fi**: Utilize um aplicativo móvel ou computador para controlar o robô.
- **Garra Robótica**: Pega e manipula objetos com precisão.
- **Seguimento de Faixas**: Sensores de linha permitem seguir trilhas no chão.
- **Desvio de Obstáculos**: Detecta e evita obstáculos no caminho.
- **Navegação Autônoma**: Algoritmos avançados de controle para movimentos suaves e precisos.

## Hardware Utilizado
### Plataformas de Desenvolvimento e Estruturas
- **Placa Vespa**
- **Plataforma Robótica**: Rocket Tank
- **Braço Robótico**: RoboARM

### Sensores
- **Sensor Ultrassônico HC-SR04** (1x)
  - Suporte para Sensor Ultrassônico
- **Sensor de Refletância QRE1113** (2x)

### Energia
- **Bateria Li-Ion 18650 3,7V** (2x)
- **Suporte para 2 Baterias de Li-Ion**
- **Carregador Duplo para Bateria Li-Ion**

### Conectores e Cabos
- **Cabo micro USB**
- **Jumpers Premium 20cm Fêmea-Fêmea** (10x)

### Outros Componentes
- **Espaçadores Sextavados de Nylon** (2x)
- **Kit de Expansão - Rocket Tank**
- **Fita de PVC** para criar traçado
- **Chave Philips**

## Montagem e Testes
### Montagem Física
1. Construa a estrutura do robô e instale a garra robótica.
2. Conecte todos os componentes eletrônicos e calibre o servo motor da garra.

### Testes de Funcionamento
1. Realize testes de operação para garantir que todos os componentes estão funcionando corretamente.
2. Faça os ajustes necessários para otimizar o desempenho do robô.

## Programação
### Ferramenta Utilizada
- **IDE**: Arduino IDE

### Desenvolvimento
- **Algoritmos de Controle**: Desenvolvimento de algoritmos para controle de movimento da garra.
- **Integração**: Módulos de comunicação Wi-Fi ou Bluetooth (opcional).
- **Protocolo MQTT**: Implementação para comunicação eficiente.

### Protocolo MQTT
O MQTT (Message Queuing Telemetry Transport) é um protocolo de comunicação leve, utilizado para conectar dispositivos IoT. Ele permite a troca de mensagens entre clientes através de um broker. Neste projeto, o MQTT é utilizado para enviar dados e receber comandos de controle do robô.

## Integração com IoT
### Conexão à Rede Wi-Fi
O robô se conecta a uma rede Wi-Fi doméstica, permitindo controle remoto.

### Desenvolvimento de Aplicativo
Um aplicativo de controle remoto será desenvolvido para que o usuário possa enviar comandos de movimento e controle da garra.

## Documentação de Código
- O código-fonte do projeto está disponível na pasta `src/`.
- A documentação do código, incluindo comentários e explicações das funções, está incluída no próprio código-fonte.

## Documentação das Interfaces e Protocolos
### Controle via Wi-Fi
- Módulo Wi-Fi conectado à rede local.
- Servidor configurado no robô para receber comandos de um dispositivo remoto.

### Garra Robótica
- Controlada por motores servo.
- Possibilidade de integrar sensores de pressão.

### Braço Robótico
- Composto por juntas articuladas controladas por motores servo.
- Utiliza sensores de posição para movimentos precisos.

### Seguimento de Faixas
- Sensores de linha detectam e seguem faixas no chão.
- Algoritmos de controle interpretam dados dos sensores e ajustam a direção.

### Desvio de Obstáculos
- Sensores de distância detectam obstáculos.
- Algoritmos de evasão calculam rotas alternativas.

### Algoritmos de Controle
- Implementação de controle PID para melhorar a estabilidade e desempenho.

## Contribuição
Contribuições são bem-vindas! Sinta-se à vontade para abrir issues e pull requests.

## Licença
Este projeto está licenciado sob a [Licença MIT](LICENSE).

---

Esperamos que este robô conectado à IoT proporcione maior autonomia e comodidade, especialmente para pessoas com dificuldades de mobilidade. Agradecemos seu interesse e participação no desenvolvimento deste projeto inovador!

## Tech Juliana

<a href="https://www.linkedin.com/in/techjuliana">
 <sub><b>Juliana Bitencourt</b></sub></a>  <a href="https://www.linkedin.com/in/techjuliana" title="LinkedIn">🚀</a>

Elaborado por Juliana Bitencourt
<br> Entre em contato!👋🏽 </br>
