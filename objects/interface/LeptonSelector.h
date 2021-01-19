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

	bool isFOtZq() const;
        bool isTighttZq() const;
	bool isFOtZqMedium0p4() const;
	bool isTighttZqMedium0p4() const;
	bool isFOtZqLoose() const;
	bool isTighttZqLoose() const;
	bool isFOttH() const;
	bool isTightttH() const;
	bool isFOOldtZq() const;
	bool isTightOldtZq() const;

        virtual bool isLooseBase() const = 0;
        virtual bool isLoose2016() const = 0;
        virtual bool isLoose2017() const = 0;
        virtual bool isLoose2018() const = 0;

	virtual bool isFOBasetZq() const = 0;
        virtual bool isFO2016tZq() const = 0;
        virtual bool isFO2017tZq() const = 0;
        virtual bool isFO2018tZq() const = 0;
	virtual bool isFOBasetZqMedium0p4() const = 0;
        virtual bool isFO2016tZqMedium0p4() const = 0;
        virtual bool isFO2017tZqMedium0p4() const = 0;
        virtual bool isFO2018tZqMedium0p4() const = 0;
	virtual bool isFOBasetZqLoose() const = 0;
        virtual bool isFO2016tZqLoose() const = 0;
        virtual bool isFO2017tZqLoose() const = 0;
        virtual bool isFO2018tZqLoose() const = 0;
        virtual bool isFOBasettH() const = 0;
        virtual bool isFO2016ttH() const = 0;
        virtual bool isFO2017ttH() const = 0;
        virtual bool isFO2018ttH() const = 0;
        virtual bool isFOBaseOldtZq() const = 0;
        virtual bool isFO2016OldtZq() const = 0;
        virtual bool isFO2017OldtZq() const = 0;
	virtual bool isFO2018OldtZq() const = 0;

	virtual bool isTightBasetZq() const = 0;
        virtual bool isTight2016tZq() const = 0;
        virtual bool isTight2017tZq() const = 0;
        virtual bool isTight2018tZq() const = 0;
	virtual bool isTightBasetZqMedium0p4() const = 0;
        virtual bool isTight2016tZqMedium0p4() const = 0;
        virtual bool isTight2017tZqMedium0p4() const = 0;
        virtual bool isTight2018tZqMedium0p4() const = 0;
	virtual bool isTightBasetZqLoose() const = 0;
        virtual bool isTight2016tZqLoose() const = 0;
        virtual bool isTight2017tZqLoose() const = 0;
        virtual bool isTight2018tZqLoose() const = 0;
        virtual bool isTightBasettH() const = 0;
        virtual bool isTight2016ttH() const = 0;
        virtual bool isTight2017ttH() const = 0;
        virtual bool isTight2018ttH() const = 0;
        virtual bool isTightBaseOldtZq() const = 0;
        virtual bool isTight2016OldtZq() const = 0;
        virtual bool isTight2017OldtZq() const = 0;
	virtual bool isTight2018OldtZq() const = 0;

        virtual bool is2016() const = 0;
        virtual bool is2017() const = 0;

    private:
        virtual LeptonSelector* clone() const & = 0;
        virtual LeptonSelector* clone() && = 0;
};
#endif
