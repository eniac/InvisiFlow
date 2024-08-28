python make.py compile main.p4 /home/yindaz/Ours/p4v16/out
python make.py switchd /home/yindaz/Ours/p4v16/main.cpp /home/yindaz/Ours/p4v16/out/ -t 0x7001 -g 100 -p 0x11

python make.py compile main.p4 /home/yindaz/High/p4v16/out
python make.py switchd /home/yindaz/High/p4v16/main.cpp /home/yindaz/High/p4v16/out/ -t 0x7001 -g 100 -p 0x11

python make.py compile main.p4 /home/yindaz/Low/p4v16/out
python make.py switchd /home/yindaz/Low/p4v16/main.cpp /home/yindaz/Low/p4v16/out/ -t 0x7001 -g 100 -p 0x11

python make.py compile main.p4 /home/yindaz/Baseline/p4v16/out
python make.py switchd /home/yindaz/Baseline/p4v16/main.cpp /home/yindaz/Baseline/p4v16/out/ -t 0x7001 -g 100 -p 0x11

python make.py compile main.p4 /home/yindaz/Test/p4v16/out
python make.py switchd /home/yindaz/Test/p4v16/main.cpp /home/yindaz/Test/p4v16/out/ -t 0x7001 -g 100 -p 0x11

python make.py compile main.p4 /home/yindaz/Mirror/p4v16/out
python make.py switchd /home/yindaz/Mirror/p4v16/main.cpp /home/yindaz/Mirror/p4v16/out/ -t 0x7001 -g 100 -p 0x11

python orbweaver.py config
python orbweaver.py debug -t 10

python make.py compile /home/leoyu/bf-sde-9.2.0/pkgsrc/p4-examples/p4_16_programs/tna_mirror/tna_mirror.p4 /home/yindaz/Mirror/p4v16/out