// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once


#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "InfluxDBUnrealClient.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLUXDBTOOLS_API UInfluxDBUnrealClient : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:

	UInfluxDBUnrealClient();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInfluxDB2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAuthorization;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Host;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Port;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OrgName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DBName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UserName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Token;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> LineProtocolBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrintDebugToLog = false;

	UFUNCTION(BlueprintCallable)
	void DumpLineProtocolBufferToDatabase();

	UFUNCTION(BlueprintCallable)	
	void AppendMeasurement(FString Label, TMap<FString,FString> Tags, TMap<FString,FString> Values);
		
	UFUNCTION(BlueprintCallable)	
	void AppendTransform(FString Label, TMap<FString,FString> Tags, FTransform Transform, bool bIncludeLocation = true, bool bIncludeRotation = true, bool bIncludeScale = true );
		
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
private:
	FString BuildURL();
	static FString BuildTimestamp();
	void AddToLineProtocolBuffer(FString Line);
	static FString BuildLineProtocol(FString Label, FString Tags, FString Values);
	void BuildAndAddLineProtocolToBuffer(FString Label, FString Tags, FString vValues);
	void PostLineProtocolToInfluxDBServer(FString Lines);
};
