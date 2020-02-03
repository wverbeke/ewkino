#ifndef SampleCrossSections_H
#define SampleCrossSections_H

#include "Sample.h"


class SampleCrossSections{

	public:
		using size_type = std::vector< double >::size_type;

		SampleCrossSections( const Sample& );

		double crossSectionRatio_pdfVar( const size_type ) const;

		double crossSectionRatio_scaleVar( const size_type ) const;
		double crossSectionRatio_MuR_1_MuF_1() const{ return crossSectionRatio_scaleVar( 1 ); }
		double crossSectionRatio_MuR_1_MuF_0p5() const{ return crossSectionRatio_scaleVar( 2 ); }
		double crossSectionRatio_MuR_2_MuF_1() const{ return crossSectionRatio_scaleVar( 3 ); }
		double crossSectionRatio_MuR_2_MuF_2() const{ return crossSectionRatio_scaleVar( 4 ); }
		double crossSectionRatio_MuR_2_MuF_0p5() const{ return crossSectionRatio_scaleVar( 5 ); }
		double crossSectionRatio_MuR_0p5_MuF_1() const{ return crossSectionRatio_scaleVar( 6 ); }
		double crossSectionRatio_MuR_0p5_MuF_2() const{ return crossSectionRatio_scaleVar( 7 ); }
		double crossSectionRatio_MuR_0p5_MuF_0p5() const{ return crossSectionRatio_scaleVar( 8 ); }


		double crossSectionRatio_psVar( const size_type ) const;
		double crossSectionRatio_ISR_InverseSqrt2() const{ return crossSectionRatio_psVar( 2 ); }
		double crossSectionRatio_FSR_InverseSqrt2() const{ return crossSectionRatio_psVar( 3 ); }
		double crossSectionRatio_ISR_Sqrt2() const{ return crossSectionRatio_psVar( 4 ); }
		double crossSectionRatio_FSR_Sqrt2() const{ return crossSectionRatio_psVar( 5 ); }
		double crossSectionRatio_ISR_0p5() const{ return crossSectionRatio_psVar( 6 ); }
		double crossSectionRatio_FSR_0p5() const{ return crossSectionRatio_psVar( 7 ); }
		double crossSectionRatio_ISR_2() const{ return crossSectionRatio_psVar( 8 ); }
		double crossSectionRatio_FSR_2() const{ return crossSectionRatio_psVar( 9 ); }
		double crossSectionRatio_ISR_0p25() const{ return crossSectionRatio_psVar( 10 ); }
		double crossSectionRatio_FSR_0p25() const{ return crossSectionRatio_psVar( 11 ); }
		double crossSectionRatio_ISR_4() const{ return crossSectionRatio_psVar( 12 ); }
		double crossSectionRatio_FSR_4() const{ return crossSectionRatio_psVar( 13 ); }

	private:
		std::vector< double > lheCrossSectionRatios;
        std::vector< double > psCrossSectionRatios;

        double crossSectionRatio_lheVar( const size_type ) const;
};


#endif
