/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "_ml.h"

const float ord_var_epsilon = FLT_EPSILON*2;
const float ord_nan = FLT_MAX*0.5f;

CvForestTree::CvForestTree()
{
    forest = NULL;
}

CvForestTree::~CvForestTree()
{
    clear();
}

bool CvForestTree::train( CvDTreeTrainData* _data, const CvMat* _subsample_idx, CvRTrees* _forest )
{
    bool result = false;

    CV_FUNCNAME( "CvForestTree::train" );

    __BEGIN__;


    clear();
    forest = _forest;

    data = _data;
    data->shared = true;
    CV_CALL(result = do_train(_subsample_idx));

    __END__;

    return result;
}

CvDTreeSplit* CvForestTree::find_best_split( CvDTreeNode* node )
{
    int vi;
    CvDTreeSplit *best_split = 0, *split = 0, *t;

    CV_FUNCNAME("CvForestTree::find_best_split");
    __BEGIN__;

    /*if( !forest )
        CV_ERROR( CV_StsNullPtr, "Invalid <forest> pointer" );*/

    CvMat* active_var_mask = 0;
    if( forest )
    {
        active_var_mask = forest->active_var_mask;
        int var_count = active_var_mask->cols;

        CV_ASSERT( var_count == data->var_count );

        for( vi = 0; vi < var_count; vi++ )
        {
            uchar temp;
            int i1 = cvRandInt(&forest->rng) % var_count;
            int i2 = cvRandInt(&forest->rng) % var_count;
            CV_SWAP( active_var_mask->data.ptr[i1],
                active_var_mask->data.ptr[i2], temp );
        }
    }
    for( vi = 0; vi < data->var_count; vi++ )
    {
        int ci = data->var_type->data.i[vi];
        if( node->num_valid[vi] <= 1 || (active_var_mask && !active_var_mask->data.ptr[vi]) )
            continue;

        if( data->is_classifier )
        {
            if( ci >= 0 )
                split = find_split_cat_gini( node, vi );
            else
                split = find_split_ord_gini( node, vi );
        }
        else
        {
            if( ci >= 0 )
                split = find_split_cat_reg( node, vi );
            else
                split = find_split_ord_reg( node, vi );
        }

        if( split )
        {
            if( !best_split || best_split->quality < split->quality )
                CV_SWAP( best_split, split, t );
            if( split )
                cvSetRemoveByPtr( data->split_heap, split );
        }
    }

    __END__;

    return best_split;
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                  Random trees                                        //
//////////////////////////////////////////////////////////////////////////////////////////

CvRTrees::CvRTrees()
{
    nclasses         = 0;
    oob_error        = 0;
    ntrees           = 0;
    trees            = NULL;
    active_var_mask  = NULL;
    var_importance   = NULL;
    proximities      = NULL;
    rng = cvRNG(0xffffffff);
}

void CvRTrees::clear()
{
    int k;
    for( k = 0; k < ntrees; k++ )
        delete trees[k];
    cvFree( (void**) &trees );
    cvReleaseMat( &active_var_mask );
    cvReleaseMat( &var_importance );
    cvReleaseMat( &proximities );
}

CvRTrees::~CvRTrees()
{
    clear();
}


bool CvRTrees::train( const CvMat* _train_data, int _tflag,
                        const CvMat* _responses, const CvMat* _var_idx,
                        const CvMat* _sample_idx, const CvMat* _var_type,
                        const CvMat* _missing_mask, CvRTParams params )
{
    bool result = false;
    CvDTreeTrainData* train_data = 0;

    CV_FUNCNAME("CvRTrees::train");
    __BEGIN__;

    int var_count = 0;

    clear();

    CvDTreeParams tree_params( params.max_depth, params.min_sample_count,
        params.regression_accuracy, params.use_surrogates, params.max_categories,
        params.cv_folds, params.use_1se_rule, false, params.priors );
    
    train_data = new CvDTreeTrainData();
    CV_CALL(train_data->set_data( _train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, tree_params, true));

    var_count = train_data->var_count;
    if( params.nactive_vars > var_count )
        params.nactive_vars = var_count;
    else if( params.nactive_vars == 0 )
        params.nactive_vars = (int)sqrt((double)var_count);
    else if( params.nactive_vars < 0 )
        CV_ERROR( CV_StsBadArg, "<nactive_vars> must be non-negative" );
    params.term_crit = cvCheckTermCriteria( params.term_crit, 0.1, 1000 );

    // Create mask of active variables at the tree nodes
    CV_CALL(active_var_mask = cvCreateMat( 1, var_count, CV_8UC1 ));
    if( params.calc_var_importance )
    {
        CV_CALL(var_importance  = cvCreateMat( 1, var_count, CV_32FC1 ));
        cvZero( var_importance );
    }
    if( params.calc_proximities )
    {
        int n = train_data->sample_count;
        CV_CALL(proximities = cvCreateMat( 1, n*(n-1)/2, CV_32FC1) );
        cvZero( proximities );
    }

    CvMat submask1, submask2;
    cvGetCols( active_var_mask, &submask1, 0, params.nactive_vars );
    cvGetCols( active_var_mask, &submask2, params.nactive_vars, var_count );
    cvSet( &submask1, cvScalar(1) );
    cvZero( &submask2 );

    CV_CALL(result = grow_forest(train_data, params.term_crit ));

    result = true;

    __END__;

    return result;
}

bool CvRTrees::grow_forest( CvDTreeTrainData* train_data, const CvTermCriteria term_crit )
{
    bool result = false;

    CvMat* sample_idx_mask_for_tree = 0;
    CvMat* sample_idx_for_tree      = 0;

    CvMat* oob_sample_votes	   = 0;
    CvMat* oob_responses       = 0;

    float* oob_samples_perm_ptr= 0;

    float* samples_ptr     = 0;
    uchar* missing_ptr     = 0;
    float* true_resp_ptr   = 0;

    CvDTreeNode** predicted_nodes_ptr = 0;

    CV_FUNCNAME("CvRTrees::grow_forest");
    __BEGIN__;

    const int max_ntrees = term_crit.max_iter;
    const double max_oob_err = term_crit.epsilon;
    
    const int dims = train_data->var_count;
    nsamples = train_data->sample_count;
    nclasses = train_data->get_num_classes();

    CvMat oob_predictions_sum = cvMat( 1, nsamples, CV_32FC1 );
    CvMat oob_num_of_predictions = cvMat( 1, nsamples, CV_32FC1 );

    trees = (CvForestTree**)cvAlloc( sizeof(CvStatModel*)*max_ntrees );
    memset( trees, 0, sizeof(CvStatModel*)*max_ntrees );

    if( train_data->is_classifier )
    {
        CV_CALL(oob_sample_votes = cvCreateMat( nsamples, nclasses, CV_32SC1 ));
        cvZero(oob_sample_votes);
    }
    else
    {
        // oob_responses[0,i] = sum of predicted values for the i-th sample
        // oob_responses[1,i] = number of summands (number of predictions for the i-th sample)
        // oob_responses[0,:] = oob_predictions_sum[:]
        // oob_responses[1,:] = oob_num_of_predictions
        CV_CALL(oob_responses = cvCreateMat( 2, nsamples, CV_32FC1 ));
        cvZero(oob_responses);
        cvGetRow( oob_responses, &oob_predictions_sum, 0 );
        cvGetRow( oob_responses, &oob_num_of_predictions, 1 );
    }
    CV_CALL(sample_idx_mask_for_tree = cvCreateMat( 1, nsamples, CV_8UC1 ));
    CV_CALL(sample_idx_for_tree      = cvCreateMat( 1, nsamples, CV_32SC1 ));
    CV_CALL(oob_samples_perm_ptr     = (float*)cvAlloc( sizeof(float)*nsamples*dims ));

    CV_CALL(samples_ptr   = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(missing_ptr   = (uchar*)cvAlloc( sizeof(uchar)*nsamples*dims ));
    CV_CALL(true_resp_ptr = (float*)cvAlloc( sizeof(float)*nsamples ));

    if( proximities )
    {
        CV_CALL(predicted_nodes_ptr  = (CvDTreeNode**)cvAlloc( sizeof(CvDTreeNode*)*nsamples ));
        memset( predicted_nodes_ptr, 0, sizeof(CvDTreeNode*)*nsamples );
    }

    CV_CALL(train_data->get_vectors( 0, samples_ptr, missing_ptr, true_resp_ptr ));

    ntrees = 0;
    while( ntrees < max_ntrees )
    {
        int i;
        int oob_samples_count = 0;

        cvZero( sample_idx_mask_for_tree );
        for( i = 0; i < nsamples; i++ ) //form sample for creation one tree
        {
            int idx = cvRandInt( &rng ) % nsamples;
            sample_idx_for_tree->data.i[i] = idx;
            sample_idx_mask_for_tree->data.ptr[idx] = 0xFF;
        }

        trees[ntrees] = new CvForestTree();
        CvForestTree* tree = trees[ntrees];
        CV_CALL(tree->train( train_data, sample_idx_for_tree, this ));

        // form array of OOB samples indices and get these samples
        CvMat sample   = cvMat( 1, dims, CV_32FC1, samples_ptr );
        CvMat missing  = cvMat( 1, dims, CV_8UC1,  missing_ptr );
        float* true_resp = true_resp_ptr;
        CvDTreeNode** predicted_nodes = predicted_nodes_ptr;

        oob_error = 0;
        float ncorrect_responses = 0; // used for estimation of variable importance
        for( i = 0; i < nsamples; i++, sample.data.fl += dims, missing.data.ptr += dims, true_resp++ )
        {
            CvDTreeNode* predicted_node = 0;
            if( proximities )
            {
                CV_CALL(predicted_node = tree->predict(&sample, &missing, true));
                *predicted_nodes++ = predicted_node;
            }
            // check if the sample is OOB
            if( sample_idx_mask_for_tree->data.ptr[i] )
                continue;

            // predict oob samples
            if( !predicted_node )
                CV_CALL(predicted_node = tree->predict(&sample, &missing, true));

            if( !train_data->is_classifier )
            {
                float resp = (float)predicted_node->value;
                oob_predictions_sum.data.fl[i] += resp;
                oob_num_of_predictions.data.fl[i] += 1;

                // compute oob error
                float avg_resp =
                    oob_predictions_sum.data.fl[i]/oob_num_of_predictions.data.fl[i];
                oob_error += powf(avg_resp - *true_resp, 2);

                ncorrect_responses += powf(resp - *true_resp, 2);
            }
            else //regression
            {
                CvMat votes;
                cvGetRow(oob_sample_votes, &votes, i);
                votes.data.i[predicted_node->class_idx]++;

                // compute oob error
                CvPoint max_loc;
                cvMinMaxLoc( &votes, 0, 0, 0, &max_loc );

                float prdct_resp = (float)train_data->cat_map->data.i[max_loc.x];
                oob_error += (fabs(prdct_resp - *true_resp) < FLT_EPSILON) ? 0 : 1;

                ncorrect_responses += ((int)predicted_node->value == (int)*true_resp);
            }
            oob_samples_count++;
        }
        if( oob_samples_count > 0 )
            oob_error /= (double)oob_samples_count;

        // estimate variable importance
        if( var_importance && oob_samples_count > 0 )
        {
            memcpy( oob_samples_perm_ptr, samples_ptr, dims*nsamples*sizeof(float));

            int m;
            for( m = 0; m < dims; m++ )
            {
                // randomly permute values of the m-th variable in the oob samples
                float* mth_var_ptr = oob_samples_perm_ptr + m;
                for( i = 0; i < nsamples; i++ )
                {
                    if( sample_idx_mask_for_tree->data.ptr[i] ) //the sample is not OOB
                        continue;
                    int i1 = cvRandInt( &rng ) % nsamples;
                    int i2 = cvRandInt( &rng ) % nsamples;
                    float temp;
                    CV_SWAP( mth_var_ptr[i1*dims], mth_var_ptr[i2*dims], temp );

                    // turn values of (m-1)-th variable, that were permuted
                    // at the previous iteration, untouched
                    if( m > 1 )
                        oob_samples_perm_ptr[i*dims+m-1] = samples_ptr[i*dims+m-1];
                }

                // predict "permuted" cases and calculate the number of votes for the
                // correct class in the variable-m-permuted oob data
                float ncorrect_responses_permuted = 0;
                CvMat sample  = cvMat( 1, dims, CV_32FC1, oob_samples_perm_ptr );
                CvMat missing = cvMat( 1, dims, CV_8UC1, missing_ptr );
                for( i = 0; i < nsamples; i++, sample.data.fl += dims, missing.data.ptr += dims )
                {
                    if( sample_idx_mask_for_tree->data.ptr[i] ) //the sample is not OOB
                        continue;
                    double predct_resp = tree->predict(&sample, &missing, true)->value;
                    float true_resp  = true_resp_ptr[i];
                    ncorrect_responses_permuted += train_data->is_classifier ?
                        (int)true_resp == (int)predct_resp
                        : powf(true_resp - (float)predct_resp, 2);
                }
                var_importance->data.fl[m] += (ncorrect_responses - ncorrect_responses_permuted);
            }
        }
        if( proximities )
        {
            int i, j;
            for( j = 1; j < nsamples; j++ )
                for( i = 0; i < j; i++ )
                    if( predicted_nodes_ptr[i] == predicted_nodes_ptr[j] )
                        proximities->data.fl[(nsamples-1)*i + j-1 - (i*(i+1))/2] += 1.f;
        }
        ntrees++;
        if( term_crit.type != CV_TERMCRIT_ITER && oob_error < max_oob_err )
            break;
    }
    if( var_importance )
        CV_CALL(cvConvertScale( var_importance, var_importance, 1./ntrees/nsamples ));
    if( proximities )
        CV_CALL(cvConvertScale( proximities, proximities, 1./ntrees ));

    result = true;

    __END__;

    cvReleaseMat( &sample_idx_mask_for_tree );
    cvReleaseMat( &sample_idx_for_tree );
    cvReleaseMat( &oob_sample_votes );
    cvReleaseMat( &oob_responses );

    cvFree( &oob_samples_perm_ptr );
    cvFree( &samples_ptr );
    cvFree( &missing_ptr );
    cvFree( &true_resp_ptr );
    cvFree( &predicted_nodes_ptr );

    return result;
}


inline const CvMat* CvRTrees::get_var_importance() const
{
    return var_importance;
}


float CvRTrees::get_proximity( int i, int j ) const
{
    float result = -1;

    CV_FUNCNAME("CvRTrees::get_proximity");
    __BEGIN__;

    if( !proximities )
        CV_ERROR( CV_StsBadArg, "The proximities were not processed" );
    if( i < 0 || i >= nsamples || j < 0 || j >= nsamples )
        CV_ERROR( CV_StsBadArg, "Indices are out of range" );

    if( i > j )
    {
        int t;
        CV_SWAP( i, j, t );
    }
    if( i == j )
        result = 1;
    else
        result = proximities->data.fl[(nsamples-1)*i + j-1 - (i*(i+1))/2];

    __END__;

    return result;
}

double CvRTrees::predict( const CvMat* sample, CvMat* missing ) const
{
    double result = -1;

    CV_FUNCNAME("CvRTrees::predict");
    __BEGIN__;

    int k;

    if( nclasses > 0 ) //classification
    {
        int max_nvotes = 0;
        int* votes = (int*)alloca( sizeof(int)*nclasses );
        memset( votes, 0, sizeof(*votes)*nclasses );
        for( k = 0; k < ntrees; k++ )
        {
            CvDTreeNode* predicted_node = trees[k]->predict( sample, missing );
            int class_idx = predicted_node->class_idx;
            CV_ASSERT( 0 <= class_idx && class_idx < nclasses );

            int nvotes = ++votes[class_idx];
            if( nvotes > max_nvotes )
            {
                max_nvotes = nvotes;
                result = predicted_node->value;
            }
        }
    }
    else // regression
    {
        result = 0;
        for( k = 0; k < ntrees; k++ )
            result += trees[k]->predict( sample, missing )->value;
        result /= (double)ntrees;
    }

    __END__;

    return result;
}


void CvRTrees::save( const char* filename, const char* name )
{
    CvFileStorage* fs = 0;
    
    CV_FUNCNAME( "CvRTrees::save" );

    __BEGIN__;

    CV_CALL( fs = cvOpenFileStorage( filename, 0, CV_STORAGE_WRITE ));
    if( !fs )
        CV_ERROR( CV_StsError, "Could not open the file storage. Check the path and permissions" );

    write( fs, name ? name : "my_random_trees" );

    __END__;

    cvReleaseFileStorage( &fs );
}


void CvRTrees::write( CvFileStorage* fs, const char* name )
{
    CV_FUNCNAME( "CvRTrees::write" );

    __BEGIN__;

    cvStartWriteStruct( fs, name, CV_NODE_MAP, CV_TYPE_NAME_ML_RTREES );

    cvWriteInt( fs, "nclasses", nclasses );
    cvWriteInt( fs, "nsamples", nsamples );
    cvWriteInt( fs, "var_count", active_var_mask->cols );
    cvWriteInt( fs, "nactive_vars", (int)cvSum(active_var_mask).val[0] );
    cvWriteReal( fs, "oob_error", oob_error );

    if( var_importance )
        cvWrite( fs, "var_importance", var_importance );
    if( proximities )
        cvWrite( fs, "proximities",    proximities );

    cvWriteInt( fs, "ntrees", ntrees );
    cvStartWriteStruct( fs, "trees", CV_NODE_SEQ );
    int k;
    for( k = 0; k < ntrees; k++ )
        trees[k]->write( fs, 0 );
    cvEndWriteStruct( fs ); //trees

    cvEndWriteStruct( fs ); //opencv-ml-rtrees

    __END__;
}


void CvRTrees::load( const char* filename, const char* name )
{
    CvFileStorage* fs = 0;
    
    CV_FUNCNAME( "CvRTrees::load" );

    __BEGIN__;

    CvFileNode* tree = 0;

    CV_CALL( fs = cvOpenFileStorage( filename, 0, CV_STORAGE_READ ));
    if( !fs )
        CV_ERROR( CV_StsError, "Could not open the file storage. Check the path and permissions" );

    if( name )
        tree = cvGetFileNodeByName( fs, 0, name );
    else
    {
        CvFileNode* root = cvGetRootFileNode( fs );
        if( root->data.seq->total > 0 )
            tree = (CvFileNode*)cvGetSeqElem( root->data.seq, 0 );
    }

    read( fs, tree );

    __END__;

    cvReleaseFileStorage( &fs );
}


void CvRTrees::read( CvFileStorage* fs, CvFileNode* fnode )
{
    CV_FUNCNAME( "CvRTrees::read" );

    __BEGIN__;

    int nactive_vars, var_count, k;
    CvSeqReader reader;
    CvMat submask1, submask2;
    CvFileNode* trees_node;

    clear();

    nclasses     = cvReadIntByName( fs, fnode, "nclasses", -1 );
    nsamples     = cvReadIntByName( fs, fnode, "nsamples" );
    var_count    = cvReadIntByName( fs, fnode, "var_count" );
    nactive_vars = cvReadIntByName( fs, fnode, "nactive_vars", -1 );
    oob_error    = cvReadRealByName(fs, fnode, "oob_error", -1 );
    ntrees       = cvReadIntByName( fs, fnode, "ntrees", -1 );

    var_importance = (CvMat*)cvReadByName( fs, fnode, "var_importance" );
    proximities    = (CvMat*)cvReadByName( fs, fnode, "proximities" );

    if( nclasses < 0 || nsamples <= 0 || nactive_vars < 0 || oob_error < 0 || ntrees <= 0 )
        CV_ERROR( CV_StsParseError, "Some <nclasses>, <nsamples>, <var_count>, "
        "<nactive_vars>, <oob_error>, <ntrees> of tags are missing" );

    rng = CvRNG( -1 );

    CV_CALL(active_var_mask = cvCreateMat( 1, var_count, CV_8UC1 ));
    
    cvGetCols( active_var_mask, &submask1, 0, nactive_vars );
    cvGetCols( active_var_mask, &submask2, nactive_vars, var_count );
    cvSet( &submask1, cvScalar(1) );
    cvZero( &submask2 );

    trees_node = cvGetFileNodeByName( fs, fnode, "trees" );
    if( !trees_node || !CV_NODE_IS_SEQ(trees_node->tag) )
        CV_ERROR( CV_StsParseError, "trees tag is missing" );

    trees = (CvForestTree**)cvAlloc( sizeof(CvStatModel*)*ntrees );
    memset( trees, 0, sizeof(CvStatModel*)*ntrees );

    cvStartReadSeq( trees_node->data.seq, &reader );
    if( reader.seq->total != ntrees )
        CV_ERROR( CV_StsParseError, "<ntrees> is not equal to the number of trees saved in file" );

    for( k = 0; k < reader.seq->total; k++ )
    {
        trees[k] = new CvForestTree();
        CV_CALL(trees[k]->read( fs, (CvFileNode*)reader.ptr ));
        trees[k]->forest = this;
        CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
    }

    __END__;
}

/* End of file. */
