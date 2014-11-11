//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "syslog.h"
#include "messmatch.h"

//---------------------------------------------------------------------------
TMessMatch::TMessMatch()
{
  OperationP = 0;
  Priority = -1;
  MatchCase = true;
  Field1 = 0;
  Contains1 = true;
  Field2 = 0;
  Contains2 = true;
}
//---------------------------------------------------------------------------
bool TMessMatch::Match(TSyslogMessage * p)
{
  if( Priority != -1 ) // filter by priority enable
    if( p->PRI != -1 ) // priority value exist in message
    {
      switch( OperationP )
      {
        case 0: // =
          if( LOG_PRI(p->PRI) != LOG_PRI(Priority) )
            return false;
        break;
        case 1: // <= (to emerg)
          if( LOG_PRI(p->PRI) > LOG_PRI(Priority) )
            return false;
        break;
        case 2: // >= (to debug)
          if( LOG_PRI(p->PRI) < LOG_PRI(Priority) )
            return false;
        break;
      }
    }

  if( ! MatchAllFilds(p, Field1, Contains1, Text1) )
    return false;
  if( ! MatchAllFilds(p, Field2, Contains2, Text2) )
    return false;
    
  return true;
}
//---------------------------------------------------------------------------
bool TMessMatch::MatchAllFilds(TSyslogMessage * p, int Field, bool Contains,
  String & Text)
{
  int l = Text.Length();
  if( l > 0 )
  {
    switch( Field )
    {
      case 0: // Text contains
      {
        bool _d = FieldContains(p->DateStr, Text);
        bool _s = FieldContains(p->SourceAddr, Text);
        bool _h = FieldContains(p->HostName, Text);
        bool _f = FieldContains(p->Facility, Text);
        bool _p = FieldContains(p->Priority, Text);
        bool _t = FieldContains(p->Tag, Text);
        bool _m = FieldContains(p->Msg, Text);
        if( Contains )
          return _d || _s || _h || _f || _p || _t || _m;
        else // NOT contains
          return !_d && !_s && !_h && !_f && !_p && !_t && !_m;
      }
      case 1: // Message text contains
      {
        bool _m = FieldContains(p->Msg, Text);
        if( Contains )
          return _m;
        else // NOT contains
          return !_m;
      }
      case 2: // IP =
        if( MatchCase )
          return Contains == (Text == p->SourceAddr);
        else
          return Contains == SameText(Text, p->SourceAddr, loUserLocale);
      case 3: // Host =
        if( MatchCase )
          return Contains == (Text == p->HostName);
        else
          return Contains == SameText(Text, p->HostName, loUserLocale);
      case 4: // Facility =
        if( MatchCase )
          return Contains == (Text == p->Facility);
        else
          return Contains == SameText(Text, p->Facility, loUserLocale);
      case 5: // Tag =
        if( MatchCase )
          return Contains == (Text == p->Tag);
        else
          return Contains == SameText(Text, p->Tag, loUserLocale);
    }
  }
  return true;
}
//---------------------------------------------------------------------------
bool TMessMatch::FieldContains(String & FieldText, String & Text)
{
  int r;
  if( MatchCase )
    r = FieldText.Pos(Text);
  else
    r = FieldText.UpperCase().Pos(Text.UpperCase());
  return r > 0;
}
//---------------------------------------------------------------------------
String TMessMatch::GetDescription(void)
{
  String rv;

  if( Priority != -1 ) // filter by priority enable
    switch( OperationP )
    {
      case 0: // =
        rv = String("Priority = ") + getcodetext(LOG_PRI(Priority), prioritynames);
      break;
      case 1: // <= (to emerg)
        rv = String("Priority from ") + getcodetext(LOG_PRI(Priority), prioritynames) +
           " to emerg";
      break;
      case 2: // >= (to debug)
        rv = String("Priority from ") + getcodetext(LOG_PRI(Priority), prioritynames) +
           " to debug";
      break;
    }

  int l = Text1.Length();
  if( l > 0 )
  {
    if( rv.Length() > 0 ) rv += " AND ";
    switch( Field1 )
    {
      case 0: // Text contains
        rv += String("Text ") + (Contains1 ? "" : "NOT ") + "contains \"" + Text1 + "\"";
      break;
      case 1: // Message text contains
        rv += String("Message ") + (Contains1 ? "" : "NOT ") + "contains \"" + Text1 + "\"";
      break;
      case 2: // IP =
        rv += String("IP") + (Contains1 ? " = " : " <> ") + "\"" + Text1 + "\"";
      break;
      case 3: // Host =
        rv += String("Host") + (Contains1 ? " = " : " <> ") + "\"" + Text1 + "\"";
      break;
      case 4: // Facility =
        rv += String("Facility") + (Contains1 ? " = " : " <> ") + "\"" + Text1 + "\"";
      break;
      case 5: // Tag =
        rv += String("Tag") + (Contains1 ? " = " : " <> ") + "\"" + Text1 + "\"";
      break;
    }
  }

  l = Text2.Length();
  if( l > 0 )
  {
    if( rv.Length() > 0 ) rv += " AND ";
    switch( Field2 )
    {
      case 0: // Text contains
        rv += String("Text ") + (Contains2 ? "" : "NOT ") + "contains \"" + Text2 + "\"";
      break;
      case 1: // Message text contains
        rv += String("Message ") + (Contains2 ? "" : "NOT ") + "contains \"" + Text2 + "\"";
      break;
      case 2: // IP =
        rv += String("IP") + (Contains2 ? " = " : " <> ") + "\"" + Text2 + "\"";
      break;
      case 3: // Host =
        rv += String("Host") + (Contains2 ? " = " : " <> ") + "\"" + Text2 + "\"";
      break;
      case 4: // Facility =
        rv += String("Facility") + (Contains2 ? " = " : " <> ") + "\"" + Text2 + "\"";
      break;
      case 5: // Tag =
        rv += String("Tag") + (Contains2 ? " = " : " <> ") + "\"" + Text2 + "\"";
      break;
    }
  }

  if( rv.Length() == 0 )
    rv = "All messages match";

  return rv;
}
//---------------------------------------------------------------------------
void TMessMatch::Save(XMLElementEx * p)
{
  p->wi("operationp", OperationP);
  p->wi("priority", Priority);
  p->wb("matchcase", MatchCase);
  p->wi("field1", Field1);
  p->wb("contains1", Contains1);
  p->ws("text1", Text1);
  p->wi("field2", Field2);
  p->wb("contains2", Contains2);
  p->ws("text2", Text2);
}
//---------------------------------------------------------------------------
void TMessMatch::Load(XMLElementEx * p)
{
  OperationP = p->ri("operationp", 0);
  Priority = p->ri("priority", -1);
  MatchCase = p->rb("matchcase", true);
  Field1 = p->ri("field1", 0);
  Contains1 = p->rb("contains1", true);
  Text1 = p->rs("text1");
  Field2 = p->ri("field2", 0);
  Contains2 = p->rb("contains2", true);
  Text2 = p->rs("text2");
}
//---------------------------------------------------------------------------
#pragma package(smart_init)

