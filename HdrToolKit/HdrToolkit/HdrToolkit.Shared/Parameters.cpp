#include "pch.h"
#include "Parameters.h"

using namespace HdrToolkit;

Parameters_Output::Parameters_Output()
{
	ForceChange();
}

Parameters_Output::Changes Parameters_Output::GetChanges()
{
	Changes changes = m_changes;
	m_changes.Set(false);
	return changes;
}

Parameters_Output::Changes Parameters_Output::PeekChanges() const
{
	return m_changes;
}

void Parameters_Output::ForceChange()
{
	m_changes.Set(true);
}

bool Parameters_Output::GetUndoDefaultGammaCorrection() const
{
	return m_undoDefaultGammaCorrection;
}

int Parameters_Output::GetOutputTexture() const
{
	return m_outputTexture;
}

bool Parameters_Output::GetShowSteps() const
{
	return m_showSteps;
}

float Parameters_Output::GetGlareIntensity() const
{
	return m_glareIntensity;
}

bool Parameters_Output::GetInvertImage() const
{
	return m_invertImage;
}

bool Parameters_Output::GetInvertColormap() const
{
	return m_invertColormap;
}

ColorMap^ Parameters_Output::GetMap() const
{
	return m_map;
}

ParameterView_Output^ Parameters_Output::Get() const
{
	ParameterView_Output^ parameters = ref new ParameterView_Output();
	parameters->UndoDefaultGammaCorrection = m_undoDefaultGammaCorrection;
	parameters->OutputTexture = m_outputTexture;
	parameters->ShowSteps = m_showSteps;
	parameters->GlareIntensity = m_glareIntensity;
	parameters->InvertImage = m_invertImage;
	parameters->InvertColormap = m_invertColormap;
	parameters->Map = m_map;
	return parameters;
}

void Parameters_Output::SetUndoDefaultGammaCorrection(bool undoGammaCorrection)
{
	if (undoGammaCorrection != m_undoDefaultGammaCorrection)
	{
		m_undoDefaultGammaCorrection = undoGammaCorrection;
		m_changes.HasChanged = true;
		m_changes.UndoDefaulGammaCorrection = true;
	}
}

void Parameters_Output::SetOutputTexture(int outputTexture)
{
	if (outputTexture != m_outputTexture)
	{
		m_outputTexture = outputTexture;
		m_changes.HasChanged = true;
		m_changes.OutputTexture = true;
	}
}

void Parameters_Output::SetShowSteps(bool showSteps)
{
	if (showSteps != m_showSteps)
	{
		m_showSteps = showSteps;
		m_changes.HasChanged = true;
		m_changes.ShowSteps = true;
	}
}

void Parameters_Output::SetGlareIntensity(float glareIntensity)
{
	if (glareIntensity != m_glareIntensity)
	{
		m_glareIntensity = glareIntensity;
		m_changes.HasChanged = true;
		m_changes.GlareIntensity = true;
	}
}

void Parameters_Output::SetInvertImage(bool invertImage)
{
	if (invertImage != m_invertImage)
	{
		m_invertImage = invertImage;
		m_changes.HasChanged = true;
		m_changes.InvertImage = true;
	}
}

void Parameters_Output::SetInvertColormap(bool invertColormap)
{
	if (invertColormap != m_invertColormap)
	{
		m_invertColormap = invertColormap;
		m_changes.HasChanged = true;
		m_changes.InvertColormap = true;
	}
}

void Parameters_Output::SetMap(ColorMap^ map)
{
	if (map != m_map)
	{
		m_map = map;
		m_changes.HasChanged = true;
		m_changes.Map = true;
	}
}

void Parameters_Output::Set(ParameterView_Output^ parameters)
{
	SetUndoDefaultGammaCorrection(parameters->UndoDefaultGammaCorrection);
	SetOutputTexture(parameters->OutputTexture);
	SetShowSteps(parameters->ShowSteps);
	SetGlareIntensity(parameters->GlareIntensity);
	SetInvertImage(parameters->InvertImage);
	SetMap(parameters->Map);
	SetInvertColormap(parameters->InvertColormap);
}

Parameters_Glare::Parameters_Glare()
{
	ForceChange();
	// Don't change wave lengths
	m_changes.Wavelengths = false;
}

Parameters_Glare::Changes Parameters_Glare::GetChanges()
{
	Changes changes = m_changes;
	m_changes.Set(false);
	return changes;
}

Parameters_Glare::Changes Parameters_Glare::PeekChanges() const
{
	return m_changes;
}

void Parameters_Glare::ForceChange()
{
	m_changes.Set(true);
}

float Parameters_Glare::GetMinWavelength() const
{
	return m_minWavelength;
}

float Parameters_Glare::GetMaxWavelength() const
{
	return m_maxWavelength;
}

ParameterView_Glare^ Parameters_Glare::Get() const
{
	ParameterView_Glare^ paramters = ref new ParameterView_Glare();
	paramters->MinWavelength = m_minWavelength;
	paramters->MaxWavelength = m_maxWavelength;
	return paramters;
}

void Parameters_Glare::SetMinWavelength(float wavelength)
{
	if (wavelength != m_minWavelength)
	{
		m_changes.HasChanged = true;
		m_changes.Wavelengths = true;
		m_minWavelength = wavelength;
	}
}

void Parameters_Glare::SetMaxWavelength(float wavelength)
{
	if (wavelength != m_maxWavelength)
	{
		m_changes.HasChanged = true;
		m_changes.Wavelengths = true;
		m_maxWavelength = wavelength;
	}
}

void Parameters_Glare::Set(ParameterView_Glare^ parameters)
{
	SetMinWavelength(parameters->MinWavelength);
	SetMaxWavelength(parameters->MaxWavelength);
}

Parameters_Blob::Parameters_Blob()
{
	ForceChange();
}

Parameters_Blob::Changes Parameters_Blob::GetChanges()
{
	Changes changes = m_changes;
	m_changes.Set(false);
	return changes;
}

Parameters_Blob::Changes Parameters_Blob::PeekChanges() const
{
	return m_changes;
}

void Parameters_Blob::ForceChange()
{
	m_changes.Set(true);
}

float Parameters_Blob::GetBlobThreshold() const
{
	return m_blobThreshold;
}

float Parameters_Blob::GetLightThreshold() const
{
	return m_lightThreshold;
}

ParameterView_Blob^ Parameters_Blob::Get() const
{
	ParameterView_Blob^ parameters = ref new ParameterView_Blob();
	parameters->BlobThreshold = m_blobThreshold;
	parameters->LightThreshold = m_lightThreshold;
	return parameters;
}

void Parameters_Blob::SetBlobThreshold(float blobThreshold)
{
	if (blobThreshold != m_blobThreshold)
	{
		m_blobThreshold = blobThreshold;
		m_changes.HasChanged = true;
		m_changes.BlobThreshold = true;
	}
}

void Parameters_Blob::SetLightThreshold(float lightThreshold)
{
	if (lightThreshold != m_lightThreshold)
	{
		m_lightThreshold = lightThreshold;
		m_changes.HasChanged = true;
		m_changes.LightThreshold = true;
	}
}

void Parameters_Blob::Set(ParameterView_Blob^ parameters)
{
	SetBlobThreshold(parameters->BlobThreshold);
	SetLightThreshold(parameters->LightThreshold);
}

Parameters_Reinhard02::Parameters_Reinhard02(float key, float sharpness, uint32_t maxLevels, float scaleStepFactor, float radiusInStandardDeviations, float threshold)
	: m_key(key)
	, m_sharpness(sharpness)
	, m_maxLevels(maxLevels)
	, m_scaleStepFactor(scaleStepFactor)
	, m_radiusInStandardDeviations(radiusInStandardDeviations)
	, m_threshold(threshold)
{
	m_changes.Set(true);
}

Parameters_Reinhard02::Changes Parameters_Reinhard02::GetChanges()
{
	Changes changes = m_changes;
	m_changes.Set(false);
	return changes;
}

void Parameters_Reinhard02::ForceChange()
{
	m_changes.Set(true);
}

float Parameters_Reinhard02::GetKey() const
{
	return m_key;
}

float Parameters_Reinhard02::GetSharpness() const
{
	return m_sharpness;
}

uint32_t Parameters_Reinhard02::GetMaxLevels() const
{
	return m_maxLevels;
}

float Parameters_Reinhard02::GetScaleStepFactor() const
{
	return m_scaleStepFactor;
}

float Parameters_Reinhard02::GetRadiusInStandardDeviations() const
{
	return m_radiusInStandardDeviations;
}

float Parameters_Reinhard02::GetThreshold() const
{
	return m_threshold;
}

ParameterView_Reinhard02^  Parameters_Reinhard02::Get() const
{
	ParameterView_Reinhard02^ parameters = ref new ParameterView_Reinhard02();
	parameters->Key = m_key;
	parameters->Sharpness = m_sharpness;
	parameters->MaxLevels = m_maxLevels;
	parameters->ScaleStepFactor = m_scaleStepFactor;
	parameters->RadiusInStandardDeviations = m_radiusInStandardDeviations;
	parameters->Threshold = m_threshold;
	return parameters;
}

void Parameters_Reinhard02::SetKey(float key)
{
	if (key != m_key)
	{
		m_key = key;
		m_changes.HasChanged = true;
		m_changes.Key = true;
	}
}

void Parameters_Reinhard02::SetSharpness(float sharpness)
{
	if (sharpness != m_sharpness)
	{
		m_sharpness = sharpness;
		m_changes.HasChanged = true;
		m_changes.Sharpness = true;
	}
}

void Parameters_Reinhard02::SetMaxLevels(uint32_t maxLevels)
{
	if (maxLevels != m_maxLevels)
	{
		m_maxLevels = maxLevels;
		m_changes.HasChanged = true;
		m_changes.MaxLevels = true;
	}
}

void Parameters_Reinhard02::SetScaleStepFactor(float scaleStepFactor)
{
	if (scaleStepFactor != m_scaleStepFactor)
	{
		m_scaleStepFactor = scaleStepFactor;
		m_changes.HasChanged = true;
		m_changes.ScaleStepFactor = true;
	}
}

void Parameters_Reinhard02::SetRadiusInStandardDeviations(float radiusInStandardDeviations)
{
	if (radiusInStandardDeviations != m_radiusInStandardDeviations)
	{
		m_radiusInStandardDeviations = radiusInStandardDeviations;
		m_changes.HasChanged = true;
		m_changes.RadiusInStandardDeviations = true;
	}
}

void Parameters_Reinhard02::SetThreshold(float threshold)
{
	if (threshold != m_threshold)
	{
		m_threshold = threshold;
		m_changes.HasChanged = true;
		m_changes.Threshold = true;
	}
}

void Parameters_Reinhard02::Set(ParameterView_Reinhard02^ parameters)
{
	SetKey(parameters->Key);
	SetSharpness(parameters->Sharpness);
	SetMaxLevels(parameters->MaxLevels);
	SetScaleStepFactor(parameters->ScaleStepFactor);
	SetRadiusInStandardDeviations(parameters->RadiusInStandardDeviations);
	SetThreshold(parameters->Threshold);
}

Parameters_Mantiuk06::Parameters_Mantiuk06(float contrastFactor, bool equalizeContrast)
	: m_contrastFactor(contrastFactor)
	, m_equalizeContrast(equalizeContrast)
{
	ForceChange();
}

Parameters_Mantiuk06::Changes Parameters_Mantiuk06::GetChanges()
{
	Changes changes = m_changes;
	m_changes.Set(false);
	return changes;
}

Parameters_Mantiuk06::Changes Parameters_Mantiuk06::PeekChanges() const
{
	return m_changes;
}

void Parameters_Mantiuk06::ForceChange()
{
	m_changes.Set(true);
}

float Parameters_Mantiuk06::GetContrastFactor() const
{
	return m_contrastFactor;
}

bool Parameters_Mantiuk06::GetEqualizeContrast() const
{
	return m_equalizeContrast;
}

ParameterView_Mantiuk06^ Parameters_Mantiuk06::Get() const
{
	ParameterView_Mantiuk06^ parameters = ref new ParameterView_Mantiuk06();
	parameters->ContrastFactor = m_contrastFactor;
	parameters->EqualizeContrast = m_equalizeContrast;

	return parameters;
}

void Parameters_Mantiuk06::SetContrastFactor(float contrastFactor)
{
	if (contrastFactor != m_contrastFactor)
	{
		m_contrastFactor = contrastFactor;
		m_changes.HasChanged = true;
		m_changes.ContrastFactor = true;
	}
}

void Parameters_Mantiuk06::SetEqualizeContrast(bool equalizeContrast)
{
	if (equalizeContrast != m_equalizeContrast)
	{
		m_equalizeContrast = equalizeContrast;
		m_changes.HasChanged = true;
		m_changes.EqualizeContrast = true;
	}
}

void Parameters_Mantiuk06::Set(ParameterView_Mantiuk06^ parameters)
{
	SetContrastFactor(parameters->ContrastFactor);
	SetEqualizeContrast(parameters->EqualizeContrast);
}

void Parameters_Output::Changes::Set(bool value)
{
	HasChanged = value;
	UndoDefaulGammaCorrection = value;
	OutputTexture = value;
	ShowSteps = value;
	GlareIntensity = value;
	InvertImage = value;
	Map = value;
}

void Parameters_Glare::Changes::Set(bool value)
{
	HasChanged = value;
	Wavelengths = value;
}

void Parameters_Blob::Changes::Set(bool value)
{
	HasChanged = value;
	BlobThreshold = value;
	LightThreshold = value;
}

void Parameters_Reinhard02::Changes::Set(bool value)
{
	HasChanged = value;
	Key = value;
	Sharpness = value;
	MaxLevels = value;
	ScaleStepFactor = value;
	RadiusInStandardDeviations = value;
	Threshold = value;
}

void Parameters_Mantiuk06::Changes::Set(bool value)
{
	HasChanged = value;
	ContrastFactor = value;
	EqualizeContrast = value;
}