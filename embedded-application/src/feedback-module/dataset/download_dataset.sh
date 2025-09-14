set -o errexit
set -o nounset
set -o pipefail 

if [ ! -e "hrir_dataset.sofa" ]; then
  echo "SOFA File does not exist. Downloading:"
  wget https://zenodo.org/records/7626148/files/KEMAR_NFHRIRmea_1cm.sofa?download=1 -O hrir_dataset.sofa
else
  echo "SOFA File exists. Ignoring..."
fi

mkdir -p piper-model
python3 -m piper.download_voices es_MX-claude-high
mv es_MX-claude-high.onnx ./piper-model
mv es_MX-claude-high.onnx.json ./piper-model
