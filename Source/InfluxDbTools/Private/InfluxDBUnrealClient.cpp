// Copyright (c) Iwer Petersen. All rights reserved.
#include "InfluxDBUnrealClient.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include <chrono>

using namespace std::chrono;

UInfluxDBUnrealClient::UInfluxDBUnrealClient(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{

}

// Sets default values for this component's properties
UInfluxDBUnrealClient::UInfluxDBUnrealClient()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UInfluxDBUnrealClient::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UInfluxDBUnrealClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInfluxDBUnrealClient::AddToLineProtocolBuffer(FString Line)
{
	LineProtocolBuffer.Add(Line);
}

void UInfluxDBUnrealClient::DumpLineProtocolBufferToDatabase()
{
	FString lines = FString::Join(LineProtocolBuffer, TEXT("\n"));
	LineProtocolBuffer.Empty();
	PostLineProtocolToInfluxDBServer(lines);
}

void UInfluxDBUnrealClient::AppendTransform(FString Label, TMap<FString,FString> Tags, FTransform Transform, bool bIncludeLocation, bool bIncludeRotation, bool bIncludeScale)
{
	TMap<FString,FString> values;

	if (bIncludeLocation)
	{
		values.Add(TEXT("location_x"), FString::SanitizeFloat(Transform.GetLocation().X));
		values.Add(TEXT("location_y"), FString::SanitizeFloat(Transform.GetLocation().Y));
		values.Add(TEXT("location_z"), FString::SanitizeFloat(Transform.GetLocation().Z));
	}
	if (bIncludeRotation)
	{
		values.Add(TEXT("rotation_x"), FString::SanitizeFloat(Transform.GetRotation().X));
		values.Add(TEXT("rotation_y"), FString::SanitizeFloat(Transform.GetRotation().Y));
		values.Add(TEXT("rotation_z"), FString::SanitizeFloat(Transform.GetRotation().Z));
		values.Add(TEXT("rotation_w"), FString::SanitizeFloat(Transform.GetRotation().W));
	}
	if (bIncludeScale)
	{
		values.Add(TEXT("scale_x"), FString::SanitizeFloat(Transform.GetScale3D().X));
		values.Add(TEXT("scale_y"), FString::SanitizeFloat(Transform.GetScale3D().Y));
		values.Add(TEXT("scale_z"), FString::SanitizeFloat(Transform.GetScale3D().Z));
	}
	AppendMeasurement(Label, Tags, values);	
}

FString UInfluxDBUnrealClient::BuildLineProtocol(FString Label, FString Tags, FString Values)
{
	return Label + "," + Tags +" " + Values + " " + BuildTimestamp();
}

void UInfluxDBUnrealClient::BuildAndAddLineProtocolToBuffer(FString Label, FString Tags, FString Values)
{
	AddToLineProtocolBuffer(BuildLineProtocol(Label, Tags, Values));
}

void UInfluxDBUnrealClient::AppendMeasurement(FString Label, TMap<FString,FString> Tags, TMap<FString,FString> Values)
{
	TArray<FString> tags;
	TArray<FString> values;

	for(auto &Elem : Tags){
		tags.Add(Elem.Key + "=" + Elem.Value);
	}
	for(auto &Elem : Values){
		values.Add(Elem.Key + "=" + Elem.Value);
	}
	
	BuildAndAddLineProtocolToBuffer(Label, FString::Join(tags, TEXT(",")), FString::Join(values, TEXT(",")));
}

void UInfluxDBUnrealClient::PostLineProtocolToInfluxDBServer(FString Lines)
{
	FString URL = BuildURL();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	if (bPrintDebugToLog)
	{
		UE_LOG(LogTemp, Warning, TEXT("Posting %s to %s"), *Lines, *URL);
		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UInfluxDBUnrealClient::OnResponseReceived);
	}
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *URL));
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	HttpRequest->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	if(bUseAuthorization)
	{
		if(bInfluxDB2)
		{
			FString Auth = TEXT("Token ") + Token;
			UE_LOG(LogTemp, Warning, TEXT("Auth header: %s"), *Auth)
			HttpRequest->SetHeader("Authorization", Auth);
		} else
		{
			FString Auth = TEXT("Basic ") + FBase64::Encode(UserName + TEXT(":") + Token);
			UE_LOG(LogTemp, Warning, TEXT("Auth header: %s"), *Auth)
			HttpRequest->SetHeader("Authorization", Auth);
		}
	} 
	
	HttpRequest->SetContentAsString(Lines);	
	HttpRequest->ProcessRequest();
}

void UInfluxDBUnrealClient::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("SUCCESS! RESPONSE IS %s"), *Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FAIL!"));
	}
}

FString UInfluxDBUnrealClient::BuildURL()
{
	FString URL;
	if(bInfluxDB2)
	{
		URL = TEXT("http://") + Host + TEXT(":") + FString::FromInt(Port) + TEXT("/api/v2/write?org=") + OrgName + TEXT("&bucket=") + DBName;
	} else
	{
		// http://165.227.123.141:8086/write?db=unrealanalytics
		URL = TEXT("http://") + Host + TEXT(":") + FString::FromInt(Port) + TEXT("/write?db=") + DBName;
	}
	return URL;
}

FString UInfluxDBUnrealClient::BuildTimestamp()
{
	nanoseconds now = duration_cast <nanoseconds>(system_clock::now().time_since_epoch());
	return FString::Printf(TEXT("%llu"), now.count());
}
