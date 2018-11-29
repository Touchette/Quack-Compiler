filename=$(basename $1)
name=${filename%.*}
./qcc $1 --json=true > $name.json
python3.6 json_to_dot.py $name.json > $name.dot
dot -Tpng $name.dot -o $name.png
rm $name.json
rm $name.dot