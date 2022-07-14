#ifndef LeptonSelector_H
#define LeptonSelector_H

class Lepton;

class LeptonSelector {
    friend class Lepton;

    public:
        bool isLoose() const;
        bool isFO() const;
        bool isTight() const;
	static std::string leptonID();

        virtual double coneCorrection() const = 0;

        virtual ~LeptonSelector(){}

    protected:

	bool isLoose_v1() const;
	bool isLoose_v2() const;
	bool isFO_v1() const;
	bool isFO_v2() const;
	bool isTight_v1() const;
	bool isTight_v2() const;

        virtual bool isLooseBase_v1() const = 0;
        virtual bool isLoose2016_v1() const = 0;
	virtual bool isLoose2016PreVFP_v1() const = 0;
	virtual bool isLoose2016PostVFP_v1() const = 0;
        virtual bool isLoose2017_v1() const = 0;
        virtual bool isLoose2018_v1() const = 0;

	virtual bool isLooseBase_v2() const = 0;
        virtual bool isLoose2016_v2() const = 0;
        virtual bool isLoose2016PreVFP_v2() const = 0;
        virtual bool isLoose2016PostVFP_v2() const = 0;
        virtual bool isLoose2017_v2() const = 0;
        virtual bool isLoose2018_v2() const = 0;

        virtual bool isFOBase_v1() const = 0;
        virtual bool isFO2016_v1() const = 0;
	virtual bool isFO2016PreVFP_v1() const = 0;
        virtual bool isFO2016PostVFP_v1() const = 0;
        virtual bool isFO2017_v1() const = 0;
        virtual bool isFO2018_v1() const = 0;

	virtual bool isFOBase_v2() const = 0;
        virtual bool isFO2016_v2() const = 0;
        virtual bool isFO2016PreVFP_v2() const = 0;
        virtual bool isFO2016PostVFP_v2() const = 0;
        virtual bool isFO2017_v2() const = 0;
        virtual bool isFO2018_v2() const = 0;

	virtual bool isFORunTime(double,double,int) const = 0;

        virtual bool isTightBase_v1() const = 0;
        virtual bool isTight2016_v1() const = 0;
	virtual bool isTight2016PreVFP_v1() const = 0;
        virtual bool isTight2016PostVFP_v1() const = 0;
        virtual bool isTight2017_v1() const = 0;
        virtual bool isTight2018_v1() const = 0;

	virtual bool isTightBase_v2() const = 0;
        virtual bool isTight2016_v2() const = 0;
        virtual bool isTight2016PreVFP_v2() const = 0;
        virtual bool isTight2016PostVFP_v2() const = 0;
        virtual bool isTight2017_v2() const = 0;
        virtual bool isTight2018_v2() const = 0;

        virtual bool is2016() const = 0;
	virtual bool is2016PreVFP() const = 0;
        virtual bool is2016PostVFP() const = 0;
        virtual bool is2017() const = 0;
	virtual bool is2018() const = 0;

    private:
        virtual LeptonSelector* clone() const & = 0;
        virtual LeptonSelector* clone() && = 0;
};
#endif
