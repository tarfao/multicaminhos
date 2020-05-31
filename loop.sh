

#Aqui verificamos se o arquivo em C foi passado por parâmetro
if test "$1"
then
for i in $(seq 1 17)
do
	./codigos/$1 $2 $3 $4
done
else
#caso contrário, abortamos a operação
echo -e
echo "Nenhum arquivo para compilar. Finalizando execução."
exit
fi

