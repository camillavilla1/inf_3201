# Simple script that runs the program using increaseingly higher amounts of threads
# Outputs to a results.out file that the including plotres.py can use to plot
# Note, not recommended to use before you've started testing

rm -f results.out

echo OpenMP
for i in `seq 1 16`
do 
    export OMP_NUM_THREADS=$i
    echo "Simple $i:"
    ./FrostTrap simple | tee -a results.out
    echo "RB $i:"
    ./FrostTrap rb | tee -a results.out
    echo "dbuf $i:"
    ./FrostTrap dbuf | tee -a results.out
done


