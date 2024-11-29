# Lírio

Injetor de Gecko para Nintendo GameCube / Decompilador PowerPC

![Preview](https://i.ibb.co/qW7mjgr/Lirio2.png)

Lírio é uma ferramenta baseada em terminal/command prompt criada para injetar, tornar permanentes, modificações do tipo Gecko (um formato de trapaça ou modificação comum em jogos), em um executável de Nintendo GameCube (.dol). Para usar, crie o arquivo code.txt, digite as linhas de código desejadas, sem espaços ou quebras de linhas extras, e execute injetar.bat. 

É possível pré-visualizar as mudanças feitas pelos códigos através do simular.bat, que demonstra a injeção mas sem realmente alterar o executável.

O programa também é um Decompilador de assembly da arquitetura PowerPC usada nos chips Gekko e Broadway, encontrados no GameCube e Wii respectivamente. Para usar, execute decompilar.bat, crie o arquivo code.txt com a string (texto) do código compilado em hexadecimal, sem espaços ou quebras entre os números, e forneça o endereço onde o código está localizado.
(Exemplo: 040C2314 38600000 injeta a instrução "li 3, 0" no endereço 800C2314)

O decompilador suporta em torno de 160 instruções do conjunto usado pelos processadores. Nem todas são reconhecidas mas a maioria do código Assembly pode ser visualizado, incluindo operações com as Paired Singles, exclusivas aos consoles da Nintendo.

Tipos de código suportados: 04 (32-bit write), 02 (16-bit write), 01 (byte write)

Testado exaustivamente, sem erros percebidos nas funções. Contudo, bugs podem ser reportados na seção de problemas do repositório, ou informe @lmfinish no Discord diretamente.

# Soluções
"Não injetável; fora dos limites do executável"
O endereço de alguma linha de código inserida não está contido em nenhuma seção do executável. Isto é comum em códigos que modificam valores ou conteúdo de arquivos escritos na memória, e a única solução é escrever uma solução em formato de Assembly no caso de valores, ou alterar o arquivo de destino diretamente no caso de arquivos.

"Tentativa de escrever X bits a endereço não alinhado"
Certifique-se de que os endereços de destino de códigos do tipo 04 estejam alinhados a 4, e endereços de códigos do tipo 02 estejam alinhados a 2. Por exemplo não é possível aplicar 4 bytes a 0xA, e não é possível aplicar 2 bytes a 0xB. Os do tipo 01 não precisam de alinhamento.

"Valor fora do limite de X bits"
Se houver uma ou mais linhas de código do tipo 02/01, verifique se o valor aplicado por elas não excede 0xFFFF/0xFF, respectivamente.

"Possíveis dados nulos ou inválidos"
Certifique-se de que code.txt contenha somente uma única string de código PowerPC compilado em hexadecimal, sem espaços ou outros caracteres inválidos.



