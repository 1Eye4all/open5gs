#!/bin/sh

export C_POST_PROCESS_FILE="/usr/bin/uncrustify --no-backup"
export UNCRUSTIFY_CONFIG=../openapi/.openapi-generator/uncrustify.cfg
openapi_generator_cli="java -jar ${HOME}/Documents/git/my/openapi-generator/modules/openapi-generator-cli/target/openapi-generator-cli.jar"

#$openapi_generator_cli generate -i ./modified/TS29503_Nudm_SDM.yaml -c ../openapi/.openapi-generator/config.yaml -g c -o ../openapi
#$openapi_generator_cli generate -i ./modified/TS29504_Nudr_DR.yaml -c ../openapi/.openapi-generator/config.yaml -g c -o ../openapi

$openapi_generator_cli generate -i ./modified/TS29503_Nudm_UECM.yaml -c ../openapi/.openapi-generator/config.yaml -g c -o ../openapi
$openapi_generator_cli generate -i ./modified/TS29503_Nudm_UEAU.yaml -c ../openapi/.openapi-generator/config.yaml -g c -o ../openapi
$openapi_generator_cli generate -i ./modified/TS29509_Nausf_UEAuthentication.yaml -c ../openapi/.openapi-generator/config.yaml -g c -o ../openapi
$openapi_generator_cli generate -i ./modified/TS29510_Nnrf_NFDiscovery.yaml -c ../openapi/.openapi-generator/config.yaml -g c -o ../openapi
$openapi_generator_cli generate -i ./modified/TS29510_Nnrf_NFManagement.yaml -c ../openapi/.openapi-generator/config.yaml -g c -o ../openapi
