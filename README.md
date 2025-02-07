# Descrição
O MemoryReaderDLL é uma biblioteca desenvolvida em C++ para leitura e manipulação de memória em tempo real, com suporte a DirectX 9. Projetado para funcionar como um In-Game Overlay, ele captura e exibe informações relevantes do jogo, proporcionando insights detalhados e facilitando a análise de dados.

## Principais Funcionalidades
* Leitura e Atualização de Memória: Captura dados do jogo em tempo real, atualizando variáveis essenciais.
* Injeção de DLL: Permite a execução de código diretamente no processo do jogo.
* Interface Gráfica Integrada (ImGui): Exibe dados na tela com um menu intuitivo e personalizável.
* Manipulação de DirectX 9: Hooks avançados para interceptação e renderização de gráficos.
* Teclas de Atalho: Ativação e desativação do menu via F10.
* Logs Detalhados: Monitoramento de valores e eventos internos do jogo.
## Como Utilizar
1. Injete a MemoryReaderDLL no processo do jogo.
2. Utilize as funções para capturar e manipular valores da memória.
3. Pressione F10 para abrir o menu interativo.
4. Monitore os logs para verificar alterações nos dados do jogo.
5. Utilize as janelas adicionais para visualizar estatísticas e depuração.
## Tecnologia Utilizada
* C++ - Código otimizado para alto desempenho.
* Windows API - Acesso e manipulação de processos e eventos.
* DirectX 9 Hooking - Interceptação de chamadas gráficas para sobreposição de interface.
* ImGui - Interface gráfica leve e integrada ao jogo.
* Manipulação de Teclado - Simulação de entrada para interagir com o jogo.
# Aviso Legal
O uso desta biblioteca é de inteira responsabilidade do usuário. A manipulação de memória de softwares de terceiros pode violar Termos de Serviço. Esta ferramenta foi desenvolvida apenas para fins educacionais e experimentais.

## Licença
Este projeto é fornecido "como está", sem garantias. Você pode modificá-lo e adaptá-lo conforme necessário.

