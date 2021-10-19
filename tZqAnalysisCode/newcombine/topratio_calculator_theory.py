#############################################################################
# very simple calculator script to calculate theory uncertainties for ratio #
#############################################################################

import math

def calculate_theory_uncertainty():
    
    # define theory cross-sections
    t_central = 59.3
    tbar_central = 31.8
    total_central = 94.2
    sum_central = t_central+tbar_central
    t_central = t_central*total_central/sum_central
    tbar_central = tbar_central*total_central/sum_central

    # define theory cross-section uncertainties per source
    # note: values must be given in per cent, as this the format in e.g. MadGraph output
    # note: tuples are in format (up,down)
    t_uncs = {'scale':(1.99,1.29),'PDF':(2.34,2.34)}
    tbar_uncs = {'scale':(2.04,1.33),'PDF':(3.18,3.18)}
    total_uncs = {'scale':(1.83,1.21),'PDF':(2.3,2.3)}

    # calculate cross-sections (also ratio) with uncertainties
    print('--- cross-sections and uncertainties ---')
    print('    (format: central, up, down)')
    
    ratio_uncs = {}
    for source in t_uncs.keys():
	mod_t_up = 1+t_uncs[source][0]/100
	mod_t_down = 1-t_uncs[source][1]/100
	mod_tbar_up = 1+tbar_uncs[source][0]/100
        mod_tbar_down = 1-tbar_uncs[source][1]/100
	per_up = (mod_t_up/mod_tbar_up-1)*100
	per_down = (mod_t_down/mod_tbar_down-1)*100
	ratio_uncs[source] = ( per_up, per_down )

    for meas in [total_uncs,t_uncs,tbar_uncs,ratio_uncs]:
	tot_up = math.sqrt(sum([(meas[key][0]/100)**2 for key in meas.keys()]))
        tot_down = math.sqrt(sum([(meas[key][1]/100)**2 for key in meas.keys()]))
	meas['total'] = (tot_up,tot_down)

    print('  inclusive (absolute):')
    print('      {} + {} - {}'.format(total_central,total_central*total_uncs['total'][0],
                                        total_central*total_uncs['total'][1]))
    print('  inclusive (relative):')
    print('      + {} % - {} %'.format(total_uncs['total'][0]*100,
                                        total_uncs['total'][1]*100))
    print('  top (absolute):')
    print('      {} + {} - {}'.format(t_central,t_central*t_uncs['total'][0],
					t_central*t_uncs['total'][1]))
    print('  top (relative):')
    print('      + {} % - {} %'.format(t_uncs['total'][0]*100,
					t_uncs['total'][1]*100))
    print('  antitop (absolute):')
    print('      {} + {} - {}'.format(tbar_central,tbar_central*tbar_uncs['total'][0],
                                        tbar_central*tbar_uncs['total'][1]))
    print('  antitop (relative):')
    print('      + {} % - {} %'.format(tbar_uncs['total'][0]*100,
                                        tbar_uncs['total'][1]*100))
    print('  ratio (absolute):')
    print('      {} + {} - {}'.format(t_central/tbar_central,
		 t_central/tbar_central*ratio_uncs['total'][0],
                 t_central/tbar_central*ratio_uncs['total'][1]))
    print('  ratio (relative):')
    print('      + {} % - {} %'.format(ratio_uncs['total'][0]*100,
                                       ratio_uncs['total'][1]*100))


if __name__=='__main__':

    calculate_theory_uncertainty()
