wget https://zenodo.org/records/7626148/files/KEMAR_NFHRIRmea_1cm.sofa?download=1 -O hrir_dataset.sofa

mkdir -p piper-model
python3 -m piper.download_voices es_MX-claude-high
mv es_MX-claude-high.onnx ./piper-model
mv es_MX-claude-high.onnx.json ./piper-model
