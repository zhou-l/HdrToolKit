#pragma once

#include "DisplayParameters.h"
#include "ColorMap.h"

namespace HdrToolkit
{
	public ref struct ParameterView_Output sealed
	{
	public:
		property bool UndoDefaultGammaCorrection;
		property int OutputTexture;
		property bool ShowSteps;
		property float GlareIntensity;
		property bool InvertImage;
		property ColorMap^ Map;
		property bool InvertColormap;
	};

	public ref struct ParameterView_Glare sealed
	{
	public:
		property float MinWavelength;
		property float MaxWavelength;
	};

	public ref struct ParameterView_Blob sealed
	{
	public:
		property float BlobThreshold;
		property float LightThreshold;
	};

	public ref struct ParameterView_Reinhard02 sealed
	{
	public:
		property float Key;
		property float Sharpness;
		property uint32_t MaxLevels;
		property float ScaleStepFactor;
		property float RadiusInStandardDeviations;
		property float Threshold;
	};

	public ref struct ParameterView_Mantiuk06 sealed
	{
	public:
		property float ContrastFactor;
		property bool EqualizeContrast;
	};

	struct Parameters_Output
	{
	public:
		struct Changes
		{
		public:
			bool HasChanged;
			bool UndoDefaulGammaCorrection;
			bool OutputTexture;
			bool ShowSteps;
			bool GlareIntensity;
			bool InvertImage;
			bool InvertColormap;
			bool Map;

			void Set(bool value);
		};

	private:
		Changes m_changes;

		bool m_undoDefaultGammaCorrection;
		int m_outputTexture;
		bool m_showSteps;
		float m_glareIntensity;
		bool m_invertImage;
		bool m_invertColormap;
		ColorMap^ m_map;

	public:
		Parameters_Output();

		Changes GetChanges();
		Changes PeekChanges() const;
		void ForceChange();

		bool GetUndoDefaultGammaCorrection() const;
		int GetOutputTexture() const;
		bool GetShowSteps() const;
		float GetGlareIntensity() const;
		bool GetInvertImage() const;
		ColorMap^ GetMap() const;
		bool GetInvertColormap() const;
		ParameterView_Output^ Get() const;


		void SetUndoDefaultGammaCorrection(bool undoGammaCorrection);
		void SetOutputTexture(int outputTexture);
		void SetShowSteps(bool showSteps);
		void SetGlareIntensity(float glareInstensity);
		void SetInvertImage(bool invertImage);
		void SetInvertColormap(bool invertColormap);
		void SetMap(ColorMap^ map);
		void Set(ParameterView_Output^ parameters);
	};

	struct Parameters_Glare
	{
	public:
		struct Changes
		{
		public: 
			bool HasChanged;
			bool Wavelengths;

			void Set(bool value);
		};

	private:
		Changes m_changes;

		float m_minWavelength;
		float m_maxWavelength;

	public:
		Parameters_Glare();

		Changes GetChanges();
		Changes PeekChanges() const;
		void ForceChange();

		float GetMinWavelength() const;
		float GetMaxWavelength() const;
		ParameterView_Glare^ Get() const;

		void SetMinWavelength(float wavelength);
		void SetMaxWavelength(float wavelength);
		void Set(ParameterView_Glare^ parameters);
	};

	struct Parameters_Blob
	{
	public:
		struct Changes
		{
		public:
			bool HasChanged;
			bool BlobThreshold;
			bool LightThreshold;

			void Set(bool value);
		};

	private:
		Changes m_changes;

		float m_blobThreshold;
		float m_lightThreshold;

	public:
		Parameters_Blob();

		Changes GetChanges();
		Changes PeekChanges() const;
		void ForceChange();

		float GetBlobThreshold() const;
		float GetLightThreshold() const;
		ParameterView_Blob^ Get() const;

		void SetBlobThreshold(float blobThreshold);
		void SetLightThreshold(float lightThreshold);
		void Set(ParameterView_Blob^ parameters);
	};

	struct Parameters_Reinhard02
	{
	public:
		struct Changes
		{
		public:
			bool HasChanged;
			bool Key;
			bool Sharpness;
			bool MaxLevels;
			bool ScaleStepFactor;
			bool RadiusInStandardDeviations;
			bool Threshold;

			void Set(bool value);
		};

	private:
		Changes m_changes;

		float m_key;
		float m_sharpness;
		uint32_t m_maxLevels;
		float m_scaleStepFactor;
		float m_radiusInStandardDeviations;
		float m_threshold;

	public:
		Parameters_Reinhard02(float key = 0.18f, float sharpness = 8.0f, uint32_t maxLevels = 8, float scaleStepFactor = 1.6f, float radiusInStandardDeviations = 2.0f, float threshold = 0.05f);

		Changes GetChanges();
		void ForceChange();

		float GetKey() const;
		float GetSharpness() const;
		uint32_t GetMaxLevels() const;
		float GetScaleStepFactor() const;
		float GetRadiusInStandardDeviations() const;
		float GetThreshold() const;
		ParameterView_Reinhard02^ Get() const;

		void SetKey(float key);
		void SetSharpness(float sharpness);
		void SetMaxLevels(uint32_t maxLevels);
		void SetScaleStepFactor(float scaleStepFactor);
		void SetRadiusInStandardDeviations(float radiusInStandardDeviations);
		void SetThreshold(float threshold);
		void Set(ParameterView_Reinhard02^ parameters);
	};

	struct Parameters_Mantiuk06
	{
	public:
		struct Changes
		{
		public:
			bool HasChanged;
			bool ContrastFactor;
			bool EqualizeContrast;

			void Set(bool value);
		};

	private:
		Changes m_changes;

		float m_contrastFactor;
		bool m_equalizeContrast;

	public:
		Parameters_Mantiuk06(float contrastFactor = 0.4f, bool equalizeContrast = false);

		Changes GetChanges();
		Changes PeekChanges() const;
		void ForceChange();

		float GetContrastFactor() const;
		bool GetEqualizeContrast() const;
		ParameterView_Mantiuk06^ Get() const;

		void SetContrastFactor(float contrastFactor);
		void SetEqualizeContrast(bool equalizeContrast);
		void Set(ParameterView_Mantiuk06^ parameters);

	};

	public ref struct ParametersView sealed
	{
	public:
		property ParameterView_Output^ Output;
		property ParameterView_Glare^ Glare;
		property ParameterView_Blob^ Blob;
		property ParameterView_Reinhard02^ Reinhard02;
		property ParameterView_Mantiuk06^ Mantiuk06;
	};
}