/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2008 Soeren Sonnenburg
 * Copyright (C) 1999-2008 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#include "lib/common.h"
#include "kernel/CustomKernel.h"
#include "features/Features.h"
#include "features/DummyFeatures.h"
#include "lib/io.h"

CCustomKernel::CCustomKernel()
: CKernel(10), kmatrix(NULL), num_rows(0), num_cols(0), upper_diagonal(false)
{
}

CCustomKernel::CCustomKernel(CKernel* k)
: CKernel(10), kmatrix(NULL), num_rows(0), num_cols(0), upper_diagonal(false)
{
	if (k->lhs_equals_rhs())
	{
		INT cols=k->get_num_vec_lhs();
		SG_DEBUG( "using custom kernel of size %dx%d\n", cols,cols);

		kmatrix= new SHORTREAL[cols*(cols+1)/2];

		upper_diagonal=true;
		num_rows=cols;
		num_cols=cols;

		for (INT row=0; row<num_rows; row++)
		{
			for (INT col=row; col<num_cols; col++)
				kmatrix[row * num_cols - row*(row+1)/2 + col]=k->kernel(row,col);
		}
	}
	else
	{
		INT rows=k->get_num_vec_lhs();
		INT cols=k->get_num_vec_rhs();
		kmatrix= new SHORTREAL[rows*cols];

		upper_diagonal=false;
		num_rows=rows;
		num_cols=cols;

		for (INT row=0; row<num_rows; row++)
		{
			for (INT col=0; col<num_cols; col++)
			{
				kmatrix[row * num_cols + col]=k->kernel(row,col);
			}
		}
	}

	dummy_init(num_rows, num_cols);

}

CCustomKernel::~CCustomKernel()
{
	cleanup();
}

SHORTREAL* CCustomKernel::get_kernel_matrix_shortreal(INT &num_vec1, INT &num_vec2, SHORTREAL* target)
{
	if (target == NULL)
		return CKernel::get_kernel_matrix_shortreal(num_vec1, num_vec2, target);
	else
	{
		num_vec1=num_rows;
		num_vec2=num_cols;
		return kmatrix;
	}
}
  
bool CCustomKernel::dummy_init(INT rows, INT cols)
{
	return init(new CDummyFeatures(rows), new CDummyFeatures(cols));
}

bool CCustomKernel::init(CFeatures* l, CFeatures* r)
{
	CKernel::init(l, r);

	SG_DEBUG( "num_vec_lhs: %d vs num_rows %d\n", l->get_num_vectors(), num_rows);
	SG_DEBUG( "num_vec_rhs: %d vs num_cols %d\n", r->get_num_vectors(), num_cols);
	ASSERT(l->get_num_vectors()==num_rows);
	ASSERT(r->get_num_vectors()==num_cols);
	return init_normalizer();
}

void CCustomKernel::cleanup_custom()
{
	delete[] kmatrix;
	kmatrix=NULL;
	upper_diagonal=false;
	num_cols=0;
	num_rows=0;
}

void CCustomKernel::cleanup()
{
	cleanup_custom();
	CKernel::cleanup();
}

bool CCustomKernel::load_init(FILE* src)
{
	return false;
}

bool CCustomKernel::save_init(FILE* dest)
{
	return false;
}

bool CCustomKernel::set_triangle_kernel_matrix_from_triangle(const DREAL* km, int len)
{
	ASSERT(km);
	ASSERT(len>0);

	INT cols = (INT) floor(-0.5 + CMath::sqrt(0.25+2*len));
	if (cols*(cols+1)/2 != len)
	{
		SG_ERROR("km should be a vector containing a lower triangle matrix, with len=cols*(cols+1)/2 elements\n");
		return false;
	}


	cleanup_custom();
	SG_DEBUG( "using custom kernel of size %dx%d\n", cols,cols);

	kmatrix= new SHORTREAL[len];

	upper_diagonal=true;
	num_rows=cols;
	num_cols=cols;

	for (INT i=0; i<len; i++)
		kmatrix[i]=km[i];

	dummy_init(num_rows, num_cols);
	return true;
}

bool CCustomKernel::set_triangle_kernel_matrix_from_full(const DREAL* km, INT rows, INT cols)
{
	ASSERT(rows==cols);

	cleanup_custom();
	SG_DEBUG( "using custom kernel of size %dx%d\n", cols,cols);

	kmatrix= new SHORTREAL[cols*(cols+1)/2];

	upper_diagonal=true;
	num_rows=cols;
	num_cols=cols;

	for (INT row=0; row<num_rows; row++)
	{
		for (INT col=row; col<num_cols; col++)
			kmatrix[row * num_cols - row*(row+1)/2 + col]=km[col*num_rows+row];
	}
	dummy_init(rows, cols);
	return true;
}

bool CCustomKernel::set_full_kernel_matrix_from_full(const DREAL* km, INT rows, INT cols)
{
	cleanup_custom();
	SG_DEBUG( "using custom kernel of size %dx%d\n", rows,cols);

	kmatrix= new SHORTREAL[rows*cols];

	upper_diagonal=false;
	num_rows=rows;
	num_cols=cols;

	for (INT row=0; row<num_rows; row++)
	{
		for (INT col=0; col<num_cols; col++)
		{
			kmatrix[row * num_cols + col]=km[col*num_rows+row];
		}
	}

	dummy_init(rows, cols);
	return true;
}
