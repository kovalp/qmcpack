//////////////////////////////////////////////////////////////////
// (c) Copyright 2013-  by Jaron T. Krogel                      //
//////////////////////////////////////////////////////////////////

#include <QMCWaveFunctions/SPOSetInputInfo.h>
#include <OhmmsData/AttributeSet.h>
#include <Utilities/unit_conversion.h>
#include <limits>

namespace qmcplusplus
{
  typedef QMCTraits::RealType RealType;

  const int      inone = numeric_limits<int>::min();
  const RealType rnone = 1e99;
  const string&  snone = "none";


  void SPOSetInputInfo::reset()
  {
    size         = inone;
    index_min    = inone;
    index_max    = inone;
    occ          = snone;
    ecut         = rnone;
    energy_min   = rnone;
    energy_max   = rnone;
    matching_tol = rnone;

    lowest_index   = inone;
    highest_index  = inone;
    lowest_energy  = rnone;
    highest_energy = rnone;
    
    has_size         = false;
    has_index_range  = false;
    has_occ          = false;
    has_ecut         = false;
    has_energy_range = false;
    has_indices      = false;
    has_energies     = false;
    has_index_info   = false;
    has_energy_info  = false;

    all_indices_computed = false;
  }


  void SPOSetInputInfo::put(xmlNodePtr cur)
  {
    using namespace Units;

    reset();

    indices.clear();
    energies.clear();

    string units_in = snone;

    OhmmsAttributeSet attrib;
    attrib.add(size,      "size"      );
    attrib.add(index_min, "index_min" );
    attrib.add(index_max, "index_max" );
    attrib.add(occ,       "occ"       );
    attrib.add(ecut,      "ecut"      );
    attrib.add(energy_min,"energy_min");
    attrib.add(energy_max,"energy_max");
    attrib.add(units_in,  "units"     );
    attrib.put(cur);

    has_size         = size!=inone;
    has_index_range  = index_min!=inone && index_max!=inone;
    has_occ          = occ!=snone;
    has_ecut         = ecut!=rnone;
    has_energy_range = energy_min!=rnone && energy_max!=rnone;
    has_indices      = false;
    has_energies     = false;

    if(has_ecut || has_energy_range)
    {
      report();
      if(units_in==snone)
        APP_ABORT("SPOSetInputInfo::put  ecut or energy range present, but units have not been provided");
      units eunits = energy_unit(units_in);
      if(has_ecut)
        ecut = convert(ecut,eunits,Ha);
      else if(has_energy_range)
      {
        energy_min = convert(energy_min,eunits,Ha);
        energy_max = convert(energy_max,eunits,Ha);
      }
    }

    xmlNodePtr element = cur->xmlChildrenNode;
    while(element!=NULL)
    {
      string ename((const char*)element->name);
      if(ename=="indices")
      {
        has_indices = true;
        putContent(indices,element);
      }
      else if(ename=="energies")
      {
        has_energies = true;
        units_in = snone;
        OhmmsAttributeSet attrib;
        attrib.add(matching_tol,"matching_tol");
        attrib.add(units_in,    "units"       );
        attrib.put(element);
        putContent(energies,element);
        if(units_in==snone)
          APP_ABORT("SPOSetInputInfo::put  energies present, but units have not been provided");
        units eunits = energy_unit(units_in);
        if(matching_tol==rnone)
          matching_tol = 1e-6;
        else
          matching_tol = convert(matching_tol,eunits,Ha);

        //convert_array(energies,eunits,Ha);
        vector<double> entmp;
        convert_array(entmp,eunits,Ha);

        sort(energies.begin(),energies.end());
      }
      element = element->next;
    }

    has_index_info  = has_size || has_index_range || has_occ || has_indices;
    has_energy_info = has_ecut || has_energy_range || has_energies; 

    find_index_extrema();

    find_energy_extrema();
  }


  void SPOSetInputInfo::find_index_extrema()
  {
    if(has_index_info)
    {
      lowest_index  = numeric_limits<int>::max();
      highest_index = numeric_limits<int>::min();
      if(has_size)
      {
        lowest_index  = min(lowest_index,0);
        highest_index = max(highest_index,size-1);
      }
      if(has_index_range)
      {
        lowest_index  = min(lowest_index, index_min);
        highest_index = max(highest_index,index_max);
      }
      if(has_occ)
      {
        int imin=-1;
        int imax=-1;
        for(int i=0;i<occ.size();++i)
          if(occ[i]=='1')
          {
            if(imin==-1)
              imin = i;
            imax = i;
          }
        if(imin!=-1)
          lowest_index  = min(lowest_index,imin);
        if(imax!=-1)
          highest_index = max(highest_index,imax);
      }
      if(has_indices)
        for(int i=0;i<indices.size();++i)
        {
          int ind = indices[i];
          lowest_index  = min(lowest_index, ind);
          highest_index = max(highest_index,ind);
        }
    }
  }


  void SPOSetInputInfo::find_energy_extrema()
  {
    if(has_energy_info)
    {
      lowest_energy  =  1e99;
      highest_energy = -1e99;
      if(has_ecut)
      {
        lowest_energy  = min(lowest_energy,-1e99);
        highest_energy = max(highest_energy,ecut);
      }
      if(has_energy_range)
      {
        lowest_energy  = min(lowest_energy, energy_min);
        highest_energy = max(highest_energy,energy_max);
      }
      if(has_energies)
        for(int i=0;i<energies.size();++i)
        {
          RealType en = energies[i];
          lowest_energy  = min(lowest_energy, en);
          highest_energy = max(highest_energy,en);
        }
    }
  }


  void SPOSetInputInfo::report(const string& pad)
  {
    app_log()<<pad<<"SPOSetInput report"<<endl;
    app_log()<<pad<<"  has_size         = "<< has_size <<endl;
    app_log()<<pad<<"  has_index_range  = "<< has_index_range <<endl;
    app_log()<<pad<<"  has_occ          = "<< has_occ <<endl;
    app_log()<<pad<<"  has_ecut         = "<< has_ecut <<endl;
    app_log()<<pad<<"  has_energy_range = "<< has_energy_range <<endl;
    app_log()<<pad<<"  has_indices      = "<< has_indices <<endl;
    app_log()<<pad<<"  has_energies     = "<< has_energies <<endl;
    app_log()<<pad<<"  size             = "<< size <<endl;
    app_log()<<pad<<"  index_min        = "<< index_min <<endl;
    app_log()<<pad<<"  index_max        = "<< index_max <<endl;
    app_log()<<pad<<"  occ              = "<< occ <<endl;
    app_log()<<pad<<"  ecut             = "<< ecut <<endl;
    app_log()<<pad<<"  energy_min       = "<< energy_min <<endl;
    app_log()<<pad<<"  energy_max       = "<< energy_max <<endl;
    app_log()<<pad<<"  # of indices     = "<<indices.size() <<endl;
    app_log()<<pad<<"  indices          = \n    ";
    for(int i=0;i<indices.size();++i)
      app_log()<<indices[i]<<" ";
    app_log()<<endl;
    app_log()<<pad<<"  # of energies    = "<<energies.size() <<endl;
    app_log()<<pad<<"  energies         = \n    ";
    for(int i=0;i<energies.size();++i)
      app_log()<<energies[i]<<" ";
    app_log()<<endl;
    app_log()<<pad<<"  matching_tol     = "<< matching_tol <<endl;
    app_log()<<pad<<"  lowest_index     = "<<lowest_index<<endl;
    app_log()<<pad<<"  highest_index    = "<<highest_index<<endl;
    app_log()<<pad<<"  lowest_energy    = "<<lowest_energy<<endl;
    app_log()<<pad<<"  highest_energy   = "<<highest_energy<<endl;
    app_log()<<pad<<"end SPOSetInput report"<<endl;
    app_log().flush();
  }



  SPOSetInputInfo::indices_t& SPOSetInputInfo::get_indices(const SPOSetInfo& states)
  {
    if(!all_indices_computed)
    {
      all_indices.clear();

      bool index_request   = has_index_range || has_occ || has_indices;
      //  || has_size   // only when legacy interface is deprecated
      bool energy_request  = has_ecut || has_energy_range || has_energies; 
      bool general_request = index_request || energy_request;

      if(general_request)
      {
        // ensure that state info has been properly intialized
        if(states.partial() || !states.has_indices() || !states.index_ordered())
          APP_ABORT("SPOSetInputInfo::get_indices\n  state info for this basis has not been properly initialized\n  this is a developer error");
        if(energy_request && !states.has_energies())
          APP_ABORT("SPOSetInputInfo::get_indices\n  energies requested for sposet\n  but energies have not been assigned to states");

        occupations.clear();

        //occupy_size(states);   // only when legacy interface is deprecated
        occupy_index_range(states);
        occupy_occ(states);
        occupy_indices(states);
        occupy_ecut(states);
        occupy_energy_range(states);
        occupy_energies(states);

        sort(all_indices.begin(),all_indices.end());
      }
      all_indices_computed = true;
    }

    return all_indices;
  }

  
  void SPOSetInputInfo::occupy_size(const SPOSetInfo& states)
  {
    if(has_size)
    {
      indices_t ind;
      for(int i=0;i<size;++i)
        ind.push_back(i);
      occupy("size",ind,states);
    }
  }

  void SPOSetInputInfo::occupy_index_range(const SPOSetInfo& states)
  {
    if(has_index_range)
    {
      indices_t ind;
      for(int i=index_min;i<index_max;++i)
        ind.push_back(i);
      occupy("index_range",ind,states);
    }
  }

  void SPOSetInputInfo::occupy_indices(const SPOSetInfo& states)
  {
    if(has_indices)
      occupy("indices",indices,states);
  }

  void SPOSetInputInfo::occupy_occ(const SPOSetInfo& states)
  {
    if(has_occ)
    {
      indices_t ind;
      for(int i=0;i<occ.size();++i)
        if(occ[i]=='1')
          ind.push_back(i);
      occupy("occ",ind,states);
    }
  }

  void SPOSetInputInfo::occupy_ecut(const SPOSetInfo& states)
  {
    if(has_ecut)
    {
      indices_t ind;
      for(int i=0;i<states.size();++i)
        if(states[i]->energy < ecut)
          ind.push_back(i);
      occupy("ecut",ind,states);
    }
  }

  void SPOSetInputInfo::occupy_energy_range(const SPOSetInfo& states)
  {
    if(has_energy_range)
    {
      indices_t ind;
      for(int i=0;i<states.size();++i)
      {
        const RealType& e = states[i]->energy;
        if(e<energy_max && e>=energy_min)
          ind.push_back(i);
      }
      occupy("energy_range",ind,states);
    }
  }

  void SPOSetInputInfo::occupy_energies(const SPOSetInfo& states)
  {
    if(has_energies)
    {
      if(!states.energy_ordered())
        APP_ABORT("SPOSetInputInfo::load_indices(energies)\n  states are not energy ordered\n  this is a developer error");
      indices_t ind;
      int i=0;
      for(int n=0;n<energies.size();++n)
      {
        RealType e = energies[n];
        bool found = false;
        while(i<states.size())
        {
          while(abs(e-states[i]->energy)<matching_tol)
          {
            ind.push_back(i);
            i++;
            found = true;
          }
          if(found)
            break;
          else
            i++;
        }
        if(!found)
          APP_ABORT("SPOSetInputInfo::load_indices(energies)\n  energy eigenvalue not found");
      }
      occupy("energies",ind,states);
    }
  }


  void SPOSetInputInfo::occupy(const string& loc,const indices_t& ind,const SPOSetInfo& states)
  {
    int imin = numeric_limits<int>::max();
    int imax = numeric_limits<int>::min();
    for(int i=0;i<ind.size();++i)
    {
      int ival = ind[i];
      imin = min(imin,ival);
      imax = max(imax,ival);
    }
    if(imin<0 || imax>=states.size())
      APP_ABORT("SPOSetInputInfo::occupy("+loc+")\n  indices are outside the range of states");
    for(int i=0;i<ind.size();++i)
    {
      int iocc = states[ind[i]]->index;
      if(iocc >= occupations.size())
      {
        int old_size = occupations.size();
        occupations.resize(iocc+1);
        for(int j=old_size;j<occupations.size();++j)
          occupations[j] = false;
      }
      if(occupations[iocc])
        APP_ABORT("SPOSetInputInfo::occupy("+loc+")\n  sposet request has overlapping index ranges");
      all_indices.push_back(iocc);
      occupations[iocc] = true;
    }
  }

}
