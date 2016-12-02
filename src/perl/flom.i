%module Flom
%{
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "flom_errors.h"
#include "flom_types.h"
#include "flom_handle.h"
%}

%include "flom_errors.h"
%include "flom_types.h"
%include "flom_handle.h"
