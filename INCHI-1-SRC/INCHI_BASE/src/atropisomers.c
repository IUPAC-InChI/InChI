

#include "inpdef.h"
#include "ichister.h"
#include "atropisomers.h"


int is_candidate_atrop_axis(inp_ATOM *at,
                            int a1, int a2,
                            QUEUE *q,
                            AT_RANK *nAtomLevel,
                            S_CHAR *cSource)
{
    int k;
    /* bond a1-a2 must be a single bond */
    int found = 0, btype = 0;
    for (k = 0; k < at[a1].valence; k++) {
        if ((int)at[a1].neighbor[k] == a2) { found = 1; btype = at[a1].bond_type[k]; break; }
    }
    if (!found || btype != 1) {
        return 0;
    }
    /* both ends need >= 2 substituents besides the axis (valence >= 3) */
    if (at[a1].valence < 3 || at[a2].valence < 3) {
        return 0;
    }
    /* NEGATIVE filter: reject if the bond sits in a small (rotation-locked) ring.
       Never require a ring - acyclic axes are valid. Bounded BFS: full cycle
       enumeration is exponential on dense graphs (metal clusters). */
    if (0 < is_bond_in_Nmax_memb_ring(at, a1, k, q, nAtomLevel, cSource,
                                      (AT_RANK) (ATROP_MIN_ROTATABLE_RING + 1))) {
        return 0;
    }
    return 1;
}

int atrop_axis_parity(inp_ATOM *at, int a1, int a2,
                      S_CHAR z_dir1[3], S_CHAR z_dir2[3])
{
    int i_next_1 = -1, i_next_2 = -1, k, dot;
    /* first neighbor of a1 that is not a2 (and vice versa) */
    for (k = 0; k < at[a1].valence; k++)
        if ((int)at[a1].neighbor[k] != a2) { i_next_1 = k; break; }
    for (k = 0; k < at[a2].valence; k++)
        if ((int)at[a2].neighbor[k] != a1) { i_next_2 = k; break; }
    if (i_next_1 < 0 || i_next_2 < 0) {
        return AB_PARITY_UNDF;
    }

    /* half-bond plane normals from geometry + wedges (no removed-H list here) */
    if (half_stereo_bond_parity(at, a1, NULL, 0, z_dir1, 0, AB_PARITY_UNDF) <= 0 ||
        half_stereo_bond_parity(at, a2, NULL, 0, z_dir2, 0, AB_PARITY_UNDF) <= 0) {
        return AB_PARITY_UNDF;
    }

    dot = triple_prod_char(at, a1, i_next_1, z_dir1, a2, i_next_2, z_dir2);
    if (dot > MIN_DOT_PROD)  return AB_PARITY_ODD;
    if (dot < -MIN_DOT_PROD) return AB_PARITY_EVEN;
    return AB_PARITY_UNDF;
}

int find_atropisomeric_atoms_and_bonds(inp_ATOM* out_at,
                                       int num_atoms,
                                       ORIG_ATOM_DATA *orig_inp_data) {

    int ret = 0;
    int i, j;
    int n_axes, idx;
    QUEUE *q;
    AT_RANK *nAtomLevel;
    S_CHAR *cSource;

    if (out_at == NULL || num_atoms <= 0) {
        return ret;
    }

    if (orig_inp_data == NULL) {
        return ret;
    }

    /* BFS work buffers for the small-ring filter */
    q = QueueCreate(num_atoms + 1, sizeof(qInt));
    nAtomLevel = (AT_RANK *) inchi_calloc(num_atoms, sizeof(nAtomLevel[0]));
    cSource = (S_CHAR *) inchi_calloc(num_atoms, sizeof(cSource[0]));
    if (q == NULL || nAtomLevel == NULL || cSource == NULL) {
        goto exit_function; /* OOM: behave as "no atropisomer" */
    }

    /* pass 1: count candidates */
    n_axes = 0;
    for (i = 0; i < num_atoms; i++)
        for (j = 0; j < out_at[i].valence; j++) {
            int nb = (int)out_at[i].neighbor[j];
            if (i < nb && is_candidate_atrop_axis(out_at, i, nb, q, nAtomLevel, cSource))
                n_axes++;
        }

    orig_inp_data->atrop_axes = NULL;
    orig_inp_data->num_atrop_axes = 0;
    if (n_axes > 0) {
        orig_inp_data->atrop_axes =
            (ATROP_AXIS *)inchi_calloc((size_t)n_axes, sizeof(ATROP_AXIS));
        if (orig_inp_data->atrop_axes == NULL) {
            goto exit_function; /* OOM: behave as "no atropisomer" */
        }
    }

    /* pass 2: fill records + set legacy flags */
    idx = 0;
    for (i = 0; i < num_atoms; i++) {
        for (j = 0; j < out_at[i].valence; j++) {
            int nb = (int)out_at[i].neighbor[j];
            if (i >= nb) continue;
            if (is_candidate_atrop_axis(out_at, i, nb, q, nAtomLevel, cSource)) {
                ATROP_AXIS *ax = &orig_inp_data->atrop_axes[idx++];
                ax->at1 = i; ax->at2 = nb;
                ax->orig_at1 = out_at[i].orig_at_number;
                ax->orig_at2 = out_at[nb].orig_at_number;
                ax->parity = (S_CHAR)atrop_axis_parity(out_at, i, nb,
                                                       ax->z_dir1, ax->z_dir2);
                orig_inp_data->bAtropisomer = 1;
                out_at[i].bAtropisomeric = 1;
                out_at[nb].bAtropisomeric = 1;
                ret = 1;
            }
        }
    }
    orig_inp_data->num_atrop_axes = idx;

exit_function:
    QueueDelete(q);
    inchi_free(nAtomLevel);
    inchi_free(cSource);

    return ret;
}
