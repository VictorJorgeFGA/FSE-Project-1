## Sobre o projeto

É uma aplicação desenvolvida em C++ para a disciplina Fundamentos de Sistema Embarcados da Universidade de Brasília no segundo semestre do ano de 2022.

## Instalação

A instalação deverá ser feita clonando este repositório através do comando

```git
git clone https://github.com/VictorJorgeFGA/FSE-Project-1.git
```

### Servidor principal

Caso se queira criar uma instancia do servidor principal, vocẽ deverá entrar na pasta `main_server` e compilar o código com o make:

```bash
make
```

### Servidor distribuído

Caso se queira criar uma instancia do servidor distribuído, você deverá entrar na pasta `distributed_server` e compilar o código com o make:

```bash
make
```

Ainda é necessário settar a configuração de pinos e endereço do servidor central. Nos arquivos `config1.json` e `config2.json` estão as configurações 1 e 2 respectivamente; consulte qual a necessidade de configuração da placa _rasp_  em que se deseja executar o servidor.

Uma vez definida qual configuração deve ser adotada, insira a mesma no arquivo de configuração padrão que será utilizado pela aplicação:

```bash
cat configN.json > config.json
```
> Ex.: `cat config2.json > config.json`

E em seguida, edite o arquivo `config.json` trocando o valor do endereço IP do servidor central adotado, alterando o valor da chave `"main_server_address"`

## Rodar a aplicação

Para rodar tanto o servidor central quanto o servidor distribuido, deve-se entrar na respectiva pasta e rodar comando `make run`. É recomendado subir primeiro o servidor central, para só depois subir o servidor distribuido, pois o servidor distribuído ainda é dependente da comunicação com o central.

## Vídeo de apresentação

Confira o vídeo [aqui](https://youtu.be/1AnR5IEDOfw)
