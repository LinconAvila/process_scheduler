import random

if __name__ == "__main__":

    print("Bem-vindo ao gerador de arquivos de entrada para o escalonador!")
    print("Escolha o algoritmo: 1: alternancia circular, 2: prioridade, 3: loteria, 4: CFS")
    alg = int(input())
    print("Informe a fracao de CPU que cada processo tera direito por vez")
    clock = int(input())
    print("Informe o numero de processos a serem criados")
    numProcessos = int(input())

    if alg == 1:
        A = "alternanciaCircular" 
    elif alg == 2:
        A = "prioridade"
    elif alg == 3:
        A = "loteria"
    elif alg == 4:
        A = "CFS"
    else:
        print("O algoritmo informado nao existe")
        exit()

    out = open("dede.txt", 'w')

    out.write(A+"|"+str(clock)+"\n")

    for i in range (0, numProcessos):
        tempo = random.randrange(1,10)*clock
        prioridade = random.randrange(1, 100)
        out.write(str(i)+"|"+str(i)+"|"+str(tempo)+"|"+str(prioridade)+"\n")

    out.close()