#include <climits>
#include <vector>
#include "TROOT.h"
#include "TRint.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TF1.h"
#include "TCutG.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TPaletteAxis.h"
#include "TPaveText.h"
#include "TObjArray.h"
#include "TMath.h"
#include "TGraph.h"
#include "TLine.h"
#include "TPolyLine.h"
#include "TMarker.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TText.h"

#include <math.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>

void pidCut_alphaLinear(int preamp,const char* treeFile,const char* cuts="&&1") {


	char polarCuts[567]="&&((-0.05>direction.CosTheta()&&direction.CosTheta()>-0.95)||(0.05<direction.CosTheta()&&direction.CosTheta()<0.95))";
	char digitCut[567]="&&nDigits>60";
	char treeCut[567];
	if(preamp==1){
		sprintf(treeCut,
			"run>100001856&&run!=100001870 %s %s %s",polarCuts,digitCut,cuts);
	}
	if(preamp==2){
		sprintf(treeCut,
			"!(-1<direction.Phi()&&direction.Phi()<0) %s %s %s",polarCuts,digitCut,cuts);
	}

	TFile* noMaskFile = new TFile(treeFile);
	TTree* noMaskTracks = (TTree*)noMaskFile->Get("tracks");
	TH2D * noMaskLvEn = new TH2D("noMaskLvEn","noMaskLvEn",4000,0,168.6,1000,0,10);
	noMaskLvEn->GetXaxis()->SetRangeUser(0,10);
	noMaskLvEn->GetYaxis()->SetRangeUser(0,10);
	noMaskLvEn->GetXaxis()->SetTitle("Track Energy [MeV]");
	noMaskLvEn->GetXaxis()->CenterTitle();
	noMaskLvEn->GetYaxis()->SetTitle("Track Length [cm]");
	noMaskLvEn->GetYaxis()->CenterTitle();

	TCanvas *c1 = new TCanvas("c1","c1",1000,700);
	char c1PlotName[234];
	// gStyle->SetOptStat(0);
	gStyle->SetPalette(55);
	gStyle->SetNumberContours(999);

	noMaskTracks->Draw("length:adc*168.6/40000>>noMaskLvEn",treeCut,"colz");

	double MeV_per_ADC=168.6/4000;
	double ADC_per_MeV=40000/168.6;
	char dumChar[123];


	/////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////  creating alpha TCutGs //////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	printf("\n\n/////////////////////// creating alpha TCutG - energy ///////////////////\n\n");
	int arrayPos=0;
	double totEnergyShift=2;
	double energyStep=0.1;
	double minShift=0;
	double minLengthCut=0.05;
	double minEnergyCut=0.03;
	int nShifts=(int)totEnergyShift/energyStep;


	noMaskLvEn->GetXaxis()->SetRangeUser(0,15);
	noMaskLvEn->GetYaxis()->SetRangeUser(0,10);
	noMaskLvEn->Draw("colz");
	TF1* alpha_pid_line = new TF1("alpha_pid_line","[1]*x+[0]",0,10);
	alpha_pid_line->SetLineWidth(3);
	alpha_pid_line->SetParameter(0,-0.2);
	alpha_pid_line->SetParameter(1,0.8);

	double alphaCutMinEnergy = alpha_pid_line->GetX(minLengthCut);
	double alphaCutMaxEnergy = alpha_pid_line->GetX(7);

	TCutG *alphaCut_energy[nShifts];
	TCutG *alphaCut_adc[nShifts];

	TF1* frag_pid_line = new TF1("frag_pid_line","[1]*x+[0]",0,100);
	frag_pid_line->SetLineWidth(5);
	frag_pid_line->SetParameter(0,1.1);
	frag_pid_line->SetParameter(1,0.04);
	double bfrag = frag_pid_line->GetParameter(0);
	double mfrag = frag_pid_line->GetParameter(1);
	double balpha = alpha_pid_line->GetParameter(0);
	double malpha = alpha_pid_line->GetParameter(1);
	double cutIntersect=(bfrag - balpha)/(malpha - mfrag);

	TCutG *fragCut_energy[nShifts];
	TCutG *fragCut_adc[nShifts];

	for (double i = 0; i < totEnergyShift; i+=energyStep){
	int cutpoint=0;

		balpha= minLengthCut-malpha*(alphaCutMinEnergy+i);
		cutIntersect=(bfrag - balpha)/(malpha - mfrag);

		sprintf(dumChar,"alphaCut_energy_preamp%d_energyShift%d",preamp,arrayPos);
		TCutG* alphaCut_energy_tmp= new TCutG(dumChar,5);
		alphaCut_energy_tmp->SetVarX("adc");
		alphaCut_energy_tmp->SetVarY("length");
		alphaCut_energy_tmp->SetLineColor(kGreen);
		alphaCut_energy_tmp->SetLineWidth(2);
		alphaCut_energy_tmp->SetFillColorAlpha(kGreen,0.2);
		alphaCut_energy_tmp->SetPoint(cutpoint,minEnergyCut,minLengthCut);cutpoint++;
		alphaCut_energy_tmp->SetPoint(cutpoint,alphaCutMinEnergy+i,minLengthCut);cutpoint++;
		alphaCut_energy_tmp->SetPoint(cutpoint,cutIntersect,frag_pid_line->Eval(cutIntersect));cutpoint++;
		alphaCut_energy_tmp->SetPoint(cutpoint,14,frag_pid_line->Eval(14));cutpoint++;
		alphaCut_energy_tmp->SetPoint(cutpoint,14,9);cutpoint++;
		alphaCut_energy_tmp->SetPoint(cutpoint,minEnergyCut,9);cutpoint++;
		alphaCut_energy_tmp->SetPoint(cutpoint,minEnergyCut,minLengthCut);cutpoint++;
		alphaCut_energy[arrayPos] = alphaCut_energy_tmp;

		cutpoint=0;
		sprintf(dumChar,"alphaCut_adc_preamp%d_adcShift%d",preamp,arrayPos);
		TCutG* alphaCut_adc_tmp= new TCutG(dumChar,5);
		alphaCut_adc_tmp->SetVarX("adc");
		alphaCut_adc_tmp->SetVarY("length");
		alphaCut_adc_tmp->SetLineColor(kGreen);
		alphaCut_adc_tmp->SetLineWidth(2);
		alphaCut_adc_tmp->SetFillColorAlpha(kGreen,0.2);
		alphaCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*minEnergyCut,minLengthCut);cutpoint++;
		alphaCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*(alphaCutMinEnergy+i),minLengthCut);cutpoint++;
		alphaCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*cutIntersect,frag_pid_line->Eval(cutIntersect));cutpoint++;
		alphaCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*14,frag_pid_line->Eval(14));cutpoint++;
		alphaCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*14,9);cutpoint++;
		alphaCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*minEnergyCut,9);cutpoint++;
		alphaCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*minEnergyCut,minLengthCut);cutpoint++;
		alphaCut_adc[arrayPos] = alphaCut_adc_tmp;
		sprintf(dumChar,"alphaCut_adc_preamp%d_energyShift%0d.C",preamp,arrayPos);
		ofstream alphaPrimitive;
		alphaPrimitive.open(dumChar);
		alphaCut_adc[arrayPos]->SavePrimitive(alphaPrimitive);

		sprintf(dumChar,"fragCut_energy_preamp%d_energyShift%d",preamp,arrayPos);
		TCutG *fragCut_energy_tmp = new TCutG(dumChar,4);
		fragCut_energy_tmp->SetVarX("adc");
		fragCut_energy_tmp->SetVarY("length");
		fragCut_energy_tmp->SetLineColor(kViolet);
		fragCut_energy_tmp->SetLineWidth(1);
		fragCut_energy_tmp->SetFillColorAlpha(kViolet,0.15);
		cutpoint=0;
		fragCut_energy_tmp->SetPoint(cutpoint,150,minLengthCut);cutpoint++;
		fragCut_energy_tmp->SetPoint(cutpoint,alphaCutMinEnergy+i,minLengthCut);cutpoint++;
		fragCut_energy_tmp->SetPoint(cutpoint,cutIntersect,frag_pid_line->Eval(cutIntersect));cutpoint++;
		fragCut_energy_tmp->SetPoint(cutpoint,14,frag_pid_line->Eval(14));cutpoint++;
		fragCut_energy_tmp->SetPoint(cutpoint,30,frag_pid_line->Eval(30));cutpoint++;
		fragCut_energy_tmp->SetPoint(cutpoint,71,3.15);cutpoint++;
		fragCut_energy_tmp->SetPoint(cutpoint,150,3.65);cutpoint++;
		fragCut_energy_tmp->SetPoint(cutpoint,150,minLengthCut);cutpoint++;\
		fragCut_energy[arrayPos] = fragCut_energy_tmp;
		fragCut_energy[arrayPos]->GetXaxis()->SetRangeUser(0,6);
		fragCut_energy[arrayPos]->GetYaxis()->SetRangeUser(0,6);
		noMaskLvEn->Draw("colz");
		fragCut_energy[arrayPos]->Draw("same");
		fragCut_energy[arrayPos]->Draw("same f");
		alphaCut_energy[arrayPos]->Draw("same");
		alphaCut_energy[arrayPos]->Draw("same f");
		sprintf(dumChar,"fragCut_energy_preamp%d_energyShift%.01f.png",preamp,i);
		c1->SaveAs(dumChar);
		sprintf(dumChar,"fragCut_energy_preamp%d_energyShift%0d.C",preamp,arrayPos);
		ofstream fragPrimitive;
		fragPrimitive.open(dumChar);
		fragCut_energy[arrayPos]->SavePrimitive(fragPrimitive);

		sprintf(dumChar,"fragCut_adc_preamp%d_adcShift%d",preamp,arrayPos);
		TCutG *fragCut_adc_tmp = new TCutG(dumChar,4);
		fragCut_adc_tmp->SetVarX("adc");
		fragCut_adc_tmp->SetVarY("length");
		fragCut_adc_tmp->SetLineColor(kViolet);
		fragCut_adc_tmp->SetLineWidth(1);
		fragCut_adc_tmp->SetFillColorAlpha(kViolet,0.15);
		cutpoint=0;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*150,minLengthCut);cutpoint++;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*alphaCutMinEnergy+i,minLengthCut);cutpoint++;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*cutIntersect,frag_pid_line->Eval(cutIntersect));cutpoint++;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*14,frag_pid_line->Eval(14));cutpoint++;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*30,frag_pid_line->Eval(30));cutpoint++;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*71,3.15);cutpoint++;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*150,3.65);cutpoint++;
		fragCut_adc_tmp->SetPoint(cutpoint,ADC_per_MeV*150,minLengthCut);cutpoint++;
		fragCut_adc[arrayPos] = fragCut_adc_tmp;

		arrayPos++;
	}


	// ///////////////////////////////////////////////////////////////////////////////////////
	// ////////////////////////////////  creating noMaskLvEn_alphas //////////////////////////
	// ///////////////////////////////////////////////////////////////////////////////////////
	// printf("\n\n/////////////////////// creating noMaskLvEn_alphas ///////////////////\n\n");
	// TH2D* noMaskLvEn_alphas[nShifts];
	// TH2D* noMaskLvEn_frags[nShifts];
	// TH2D* noMaskLvEn_alphas_tmp = new TH2D("noMaskLvEn_alphas_tmp",dumChar,40000,0,40000,1000,0,10);
	// TH2D* noMaskLvEn_frags_tmp = new TH2D("noMaskLvEn_frags_tmp",dumChar,40000,0,40000,1000,0,10);
	// arrayPos=0;
	// noMaskFile->cd();

	// for (double i = 0; i < totEnergyShift; i+=energyStep){
	// 	printf("/////////////// nEnergyShift %.01f ///////////////\n",i);
	// 	sprintf(dumChar,"noMaskLvEn_alphas_energyShift%.01f",i);
	// 	noMaskLvEn_alphas_tmp->SetTitle(dumChar);
	// 	noMaskLvEn_alphas_tmp->SetName(dumChar);
	// 	char alphaCut_adc_char[345];
	// 	sprintf(alphaCut_adc_char,"%s && %s",treeCut,alphaCut_adc[arrayPos]->GetName());
	// 	printf("%s\n", alphaCut_adc_char);
	// 	noMaskTracks->Draw("length:adc>>noMaskLvEn_alphas_tmp",alphaCut_adc_char,"colz");
	// 	noMaskLvEn_alphas[arrayPos]=noMaskLvEn_alphas_tmp;

	// 	noMaskLvEn_alphas_tmp->GetXaxis()->SetRangeUser(0,6000);
	// 	noMaskLvEn_alphas_tmp->GetYaxis()->SetRangeUser(0,10);
	// 	noMaskLvEn_alphas_tmp->Draw("colz");
	// 	c1->SetLogz(1);c1->SetLogx(0);c1->Update();
	// 	sprintf(dumChar,"noMaskLvEn_alphas_energyShift%d.png",arrayPos);
	// 	c1->SaveAs(dumChar);

	// 	sprintf(dumChar,"noMaskLvEn_frags_energyShift%.01f",i);
	// 	noMaskLvEn_frags_tmp->SetTitle(dumChar);
	// 	noMaskLvEn_frags_tmp->SetName(dumChar);
	// 	char fragCut_adc_char[345];
	// 	sprintf(fragCut_adc_char,"%s && %s",treeCut,fragCut_adc[arrayPos]->GetName());
	// 	printf("%s\n", fragCut_adc_char);
	// 	noMaskTracks->Draw("length:adc>>noMaskLvEn_frags_tmp",fragCut_adc_char,"colz");
	// 	noMaskLvEn_frags[arrayPos]=noMaskLvEn_frags_tmp;

	// 	noMaskLvEn_frags_tmp->GetXaxis()->SetRangeUser(0,6000);
	// 	noMaskLvEn_frags_tmp->GetYaxis()->SetRangeUser(0,10);
	// 	noMaskLvEn_frags_tmp->Draw("colz");
	// 	sprintf(dumChar,"noMaskLvEn_frags_energyShift%d.png",arrayPos);
	// 	c1->SaveAs(dumChar);
	
	// 	arrayPos++;
	// }

	///////////////////////////////////////////////////////////////////////////
	////////////////////////////////  plotting TCutGs  ////////////////////////
	///////////////////////////////////////////////////////////////////////////
	printf("\n\n/////////////////////// plotting TCutGs ///////////////////\n\n");
	sprintf(dumChar,"$cft/pid/pidCut_alphaLinear%d.root",preamp);
	TFile* outfile = new TFile(dumChar,"recreate");
	outfile->cd();

	c1->SetLogz(1);
	arrayPos=0;	
	for (double i = 0; i < totEnergyShift; i+=energyStep){
	
		noMaskLvEn->GetXaxis()->SetRangeUser(0,15);
		noMaskLvEn->GetYaxis()->SetRangeUser(0,10);
		noMaskLvEn->Draw("Colz");
		fragCut_energy[arrayPos]->Draw("same f");
		fragCut_energy[arrayPos]->Draw("same");
		alphaCut_energy[arrayPos]->Draw("same f");
		alphaCut_energy[arrayPos]->Draw("same");
		sprintf(dumChar,"bothCuts_noMaskLvEn_energyShift%0.01f.png",i);
		c1->SaveAs(dumChar);

		// noMaskLvEn_alphas[arrayPos]->GetXaxis()->SetRangeUser(0,15);
		// noMaskLvEn_alphas[arrayPos]->GetYaxis()->SetRangeUser(0,10);
		// noMaskLvEn_alphas[arrayPos]->Draw("Colz");
		// alphaCut_energy[arrayPos]->Draw("same");
		// alphaCut_energy[arrayPos]->Draw("same f");
		// sprintf(dumChar,"noMaskLvEn_alphas_energyShift%0.01f.png",i);
		// c1->SaveAs(dumChar);

		// noMaskLvEn_frags[arrayPos]->GetXaxis()->SetRangeUser(0,5);
		// noMaskLvEn_frags[arrayPos]->GetYaxis()->SetRangeUser(0,3);
		// noMaskLvEn_frags[arrayPos]->Draw("Colz");
		// fragCut_energy[arrayPos]->Draw("same");
		// fragCut_energy[arrayPos]->Draw("same f");
		// sprintf(dumChar,"noMaskLvEn_frags_energyShift%0.01f.png",i);
		// c1->SaveAs(dumChar);

		alphaCut_energy[arrayPos]->Write();
		fragCut_energy[arrayPos]->Write();
		alphaCut_adc[arrayPos]->Write();
		fragCut_adc[arrayPos]->Write();
		// noMaskLvEn_alphas[arrayPos]->Write();
		// noMaskLvEn_frags[arrayPos]->Write();

		arrayPos++;
	}

	outfile->Close();

}
