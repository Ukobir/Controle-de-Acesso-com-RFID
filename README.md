# 

<h1 align="center">
  <br>
    <img width="200px" src="https://github.com/Ukobir/Controle-de-Acesso/blob/main/imagens/logo.png">
  <br>
  Controle de Acesso à bilioteca
  <br>
</h1>

## Descrição

Nesta atividade, foi desenvolvido um código para a Bitdoglab que permite automatizar o controle do número de pessoas permitidas em um local. Neste caso, escolheu-se uma biblioteca. Para isso, o sistema utiliza um display para mostrar o número total de vagas, bem como as vagas ocupadas e restantes.

O sistema conta com sinalização luminosa, por meio dos LEDs RGB e da Matriz de LED, e sinalização sonora, utilizando Buzzers. Quando uma pessoa entra no local, deve-se pressionar o Botão A, que reduz em 1 o número de vagas disponíveis na biblioteca. Ao sair, deve-se apertar o Botão B, fazendo com que o número de vagas disponíveis aumente em 1.

## Atualização

Adição do sensor RFID MFRC522 para o controle de acesso.


### Pré-requisitos

1. **Git**: Certifique-se de ter o Git instalado no seu sistema. 
2. **VS Code**: Instale o Visual Studio Code, um editor de código recomendado para desenvolvimento com o Raspberry Pi Pico.
3. **Pico SDK**: Baixe e configure o SDK do Raspberry Pi Pico, conforme as instruções da documentação oficial.
4. Tenha acesso a placa **Bitdoglab**.
5. Tenha acesso um sensor RFID MFRC522 com as tags ou cartões.

### Passos para Execução

1. **Clonar o repositório**: Clone o repositório utilizando o comando Git no terminal:
   
   ```bash
   https://github.com/Ukobir/Controle-de-Acesso.git
   ```
2. Importar o código no VS Code com a extensão do Raspberry pi pico.
3. Compilar o código e carregar o código na **BitDogLab**.

## Testes Realizados
Foi feito diversos testes para garantir a funcionamento devido da atividade. Além de que foi organizado o código conforme explicado em aula.

## Vídeo de Demonstração
[Link do Vídeo](https://drive.google.com/file/d/16TwoUD39xuKj7zJQTWyzXFkHQf-Ilcuq/view?usp=sharing)


