#pragma once

/*
	Simulation class code is Pawel Wysocki's property.
	You are not allowed to use, copy or distribute it.
*/
class Simulation
{

	//stale do prawdopodobienstw 
	double const_Phit = 0.04;
	double b1_Pb = 0.001;
	double b2_Pb = 300.0;
	double t_Pd = 0.002;
	double a_Pd = 3 * pow(10, -7);
	double n_Pd = 3.0;
	double t_Pmd = 0.003;
	double a_Pmd = 7 * pow(10, -4);
	double n_Pmd = 3.0;
	double t_Pm = pow(10, -4);
	double a_Pm = pow(10, -8);
	double n_Pm = 3.0;
	double q_Pr = pow(10, -4);
	double a_Pr = pow(10, -8);
	double n_Pr = 3.0;
	double const_Prm = 0.5;
	double a_Prc = 0.0001;
	double n_Prc = 4.0;
	double const_Prd = 0.00006;
	double const_Pcrd = 0.003;
	double a1_Pa = 1.0;
	double a2_Pa = 100.0;
	double a3_Pa = 1.0;

	double Pcd = 0.001;


	double Phit(double D)
	{
		return 1 - exp(-const_Phit * D);
	}

	double Pb1(double D)
	{
		return b1_Pb * (1 - exp(-b2_Pb * D));
	}

	//CHECK
	double Pd(uint16_t time)
	{
		return (1 - t_Pd)*(1 - exp(-a_Pd * pow(time, n_Pd))) + t_Pd;
	}

	//CHECK
	double Pmd(uint16_t time)
	{
		return (1 - t_Pmd)*(1 - exp(-a_Pmd * pow(time, n_Pmd))) + t_Pmd;
	}

	double Pm(uint16_t time)
	{
		return (1 - t_Pm)*(1 - exp(-a_Pm * pow(time, n_Pm))) + t_Pm;
	}

	double Pr(uint16_t time)
	{
		return q_Pr * exp(-a_Pr * pow(time, n_Pr));
	}

	double Prm(double D)
	{
		return 1 - exp(-const_Prm * D);
	}

	//CHECK
	double Prc(uint8_t mutationCounter)
	{
		return 1 - exp(-a_Prc * pow(mutationCounter, n_Prc));
	}

	double Prd(double D)
	{
		return 1 - exp(-const_Prd * D);
	}

	double Pcrd(double D)
	{
		return 1 - exp(-const_Pcrd * D);
	}

	//CHECK
	double Pa(double D)
	{
		return 0.25*a1_Pa*a3_Pa*a3_Pa*D*D*exp(2.0 - a2_Pa * D - a3_Pa);
	}

public:

	void Simulate(std::vector<cellDataStruct*> &organizm, double dawka, int time, ndataPacket* packet)
	{
		double losowa1, losowa2, losowa3, losowa4;
		int zdrowe = 0, zmutowane = 0, nowotworowe = 0, martwe = 0;

		for (int i = 0; i < organizm.size(); i++)
		{
			losowa1 = double(rand()) / (RAND_MAX);
			//organizm[i]->lifeTime++;

			if (organizm[i]->state == 0)
			{
				//zdrowe++;
				if (losowa1 <= Phit(dawka)) //jeœli zdrowa trafiona 
				{
					losowa2 = double(rand()) / (RAND_MAX);
					losowa3 = double(rand()) / (RAND_MAX);
					losowa4 = double(rand() % organizm.size());
					if (losowa2 <= Prd(dawka) + Pd(organizm[i]->lifeTime))
					{
						organizm[i]->state = -1;
					} //œmieræ komórki
					if (losowa2 >= Pd(organizm[i]->lifeTime) + Prd(dawka) &&
						losowa2 <= Pd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime))
					{
						organizm[i]->state = 1; organizm[i]->mutationCounter++;
						if (losowa3 <= Prc(organizm[i]->mutationCounter))
						{
							organizm[i]->state = 2;
						}
					} //dodatkowa spontaniczna mutacja
					if (losowa2 >= Pd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) &&
						losowa2 <= Pd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pb1(dawka))
					{
						organizm[(int)losowa4]->mutationCounter++;
					}
					if (losowa2 >= Pd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pb1(dawka) && losowa2 <= Pd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pb1(dawka) + Prm(dawka)) {
						organizm[i]->state = 1; organizm[i]->mutationCounter++;
						if (losowa3 <= Prc(organizm[i]->mutationCounter)) { organizm[i]->state = 2; }
					} //dodatkowa spontaniczna mutacja
				}
				if (losowa1 >= Phit(dawka)) //jeœli zdrowa nietrafiona
				{
					losowa2 = double(rand()) / (RAND_MAX);
					losowa3 = double(rand()) / (RAND_MAX);
					if (losowa2 <= Pd(organizm[i]->lifeTime)) { organizm[i]->state = -1; }; //œmieræ komórki
					if (losowa2 >= Pd(organizm[i]->lifeTime) && losowa2 <= Pd(organizm[i]->lifeTime) + Pm(organizm[i]->lifeTime)) {
						organizm[i]->state = 1; organizm[i]->mutationCounter++;
						if (losowa3 <= Prc(organizm[i]->mutationCounter)) { organizm[i]->state = 2; }
					} //dodatkowa spontaniczna mutacja
				}
			}
			if (organizm[i]->state == 1) //jesli zmutowana 
			{
				//zmutowane++;
				if (losowa1 <= Phit(dawka)) //jeœli zmutowana trafiona 
				{
					losowa2 = double(rand()) / (RAND_MAX);
					losowa3 = double(rand()) / (RAND_MAX);
					losowa4 = double(rand() % organizm.size());
					if (losowa2 <= Pmd(organizm[i]->lifeTime) + Prd(dawka))
					{
						organizm[i]->state = -1;
					} //smieræ 
					if (losowa2 >= Pmd(organizm[i]->lifeTime) + Prd(dawka) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime))
					{
						organizm[i]->state = 1; organizm[i]->mutationCounter++;
						if (losowa3 <= Prc(organizm[i]->mutationCounter))
						{
							organizm[i]->state = 2;
						}
					} //dodatkowa spontaniczna mutacja
					if (losowa2 >= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pa(i))
					{
						organizm[i]->mutationCounter--;
					}//odpowiedŸ adaptacyjna
					if (losowa2 >= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pa(i) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pa(i) + Pb1(dawka))
					{
						organizm[(int)losowa4]->mutationCounter++;
					}
					if (losowa2 >= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pa(i) + Pb1(dawka) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime)
						+ Pa(i) + Pb1(dawka) + Pr(i))
					{
						organizm[i]->mutationCounter--;
					}//naturalna reperacja jednej z mutacji
					if (losowa2 >= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pa(i) + Pb1(dawka) + Pr(i) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Prd(dawka) + Pm(organizm[i]->lifeTime) + Pa(i) + Pb1(dawka) + Pr(i) + Prm(dawka))
					{
						organizm[i]->mutationCounter++;
						if (losowa3 <= Prc(organizm[i]->mutationCounter))
						{
							organizm[i]->state = 2;
						}
					}//doatkowa spontaniczna mutacja
				}
				if (losowa1 >= Phit(dawka)) //jeœli zmutowana nietrafiona 
				{
					losowa2 = double(rand()) / (RAND_MAX);
					losowa3 = double(rand()) / (RAND_MAX);
					if (losowa2 <= Pmd(organizm[i]->lifeTime))
					{
						organizm[i]->state = -1;
					} //smieræ 
					if (losowa2 >= Pmd(organizm[i]->lifeTime) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Pm(organizm[i]->lifeTime))
					{
						organizm[i]->state = 1; organizm[i]->mutationCounter++;
						if (losowa3 <= Prc(organizm[i]->mutationCounter))
						{
							organizm[i]->state = 2;
						}
					} //dodatkowa spontaniczna mutacja
					if (losowa2 >= Pmd(organizm[i]->lifeTime) + Pm(organizm[i]->lifeTime) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Pm(organizm[i]->lifeTime) + Pa(i))
					{
						organizm[i]->mutationCounter--;
					}//odpowiedŸ adaptacyjna
					if (losowa2 >= Pmd(organizm[i]->lifeTime) + Pm(organizm[i]->lifeTime) + Pa(i) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Pm(organizm[i]->lifeTime) + Pa(i) + Pr(i))
					{
						organizm[i]->mutationCounter--;
					}
				}
			}
			if (organizm[i]->state == 2) //jesli nowotworowa 
			{
				//nowotworowe++;
				if (losowa1 <= Phit(dawka)) //jeœli nowotworowa trafiona 
				{
					losowa2 = double(rand()) / (RAND_MAX);
					losowa3 = double(rand()) / (RAND_MAX);
					losowa4 = double(rand() % organizm.size());
					if (losowa2 <= Prd(dawka) + Pcd)
					{
						organizm[i]->state = -1;
					} //smieræ 
					if (losowa2 >= Prd(dawka) + Pcd && losowa2 <= Pmd(organizm[i]->lifeTime) + Pcd + Pcrd(dawka))
					{
						organizm[i]->state = -1;
					}
					if (losowa2 >= Prd(dawka) + Pcd + Pcrd(dawka) &&
						losowa2 <= Pmd(organizm[i]->lifeTime) + Pcd + Pcrd(dawka) + Pb1(dawka))
					{
						organizm[(int)losowa4]->mutationCounter++;
					}


				}
				if (losowa1 >= Phit(dawka)) //jeœli nowotworowa nietrafiona 
				{
					losowa2 = double(rand()) / (RAND_MAX);
					losowa3 = double(rand()) / (RAND_MAX);
					if (losowa2 <= Pcd)
					{
						organizm[i]->state = -1;
					} //smieræ 
				}
			}
			switch (organizm[i]->getState())
			{
			case 0:
				zdrowe++;
				break;
			case 1:
				zmutowane++;
				break;
			case 2:
				nowotworowe++;
				break;
			case -1:
			case 255:
				martwe++;
				break;
			}
			organizm[i]->lifeTime++;
		}
		packet->healthy = zdrowe;
		packet->mutated = zmutowane;
		packet->canceroues = nowotworowe;
		packet->dead = martwe;
		packet->citeration = time + 1;
		//	std::cout << "zdrowe: " << zdrowe << "	zmutowane: " << zmutowane << "	rakowe: " << nowotworowe << "	martwe: " << martwe << " t =  " << organizm[0]->lifeTime << std::endl;
	}

	Simulation();
	~Simulation();
};

