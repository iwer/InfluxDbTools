# InfluxDbTools

The InfluxDbTools plugin has a very tiny, very simple job to help you push data to your database painlessly, or at least somewhat easily.

Originally from [Calben](https://github.com/calben/UnrealInfluxDbDemos). Upgraded for UE 4.26.

## Funding
Development of this code was partially funded by the German Federal Ministry of Education and Research (BMBF) under funding reference number 13FH1I01IA.

## Influx + Grafana on Docker

### Install
Windows:
Install Docker Desktop +  Windows Subsystem for Linux (WSL2)

Powershell:
```powershell
wsl --set-default-version 2
```

### Pull images
```powershell
docker run -d -p 8086:8086 --name=influxdb library/influxdb

docker run -d -p 3000:3000 --name=grafana grafana/grafana
```
### Start bash on influx
```powershell
docker exec -u 0 -it influxdb bash
```
```bash
$ influx setup
```
### Browser
InfluxDB http://localhost:8086

Grafana  http://localhost:3000

### Create v1 api
```bash
$ influx bucket list
$ influx v1 auth create --write.bucket bucketid --username ue4 --password research
$ influx v1 dbrp create --bucket-id 9f48215839f6eb6a --db research --rp 0
```
