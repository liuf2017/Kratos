// Authors: J. Irazábal (CIMNE)
// Date: April 2019

#include "DEM_D_Stress_dependent_cohesive_CL.h"
#include "custom_elements/spheric_particle.h"

namespace Kratos {

    void DEM_D_Stress_Dependent_Cohesive::Initialize(const ProcessInfo& r_process_info) {}

    DEMDiscontinuumConstitutiveLaw::Pointer DEM_D_Stress_Dependent_Cohesive::Clone() const {
        DEMDiscontinuumConstitutiveLaw::Pointer p_clone(new DEM_D_Stress_Dependent_Cohesive(*this));
        return p_clone;
    }

    void DEM_D_Stress_Dependent_Cohesive::SetConstitutiveLawInProperties(Properties::Pointer pProp, bool verbose) const {
        if(verbose) KRATOS_INFO("DEM") << "Assigning DEM_D_Stress_Dependent_Cohesive to Properties " << pProp->Id() << std::endl;
        pProp->SetValue(DEM_DISCONTINUUM_CONSTITUTIVE_LAW_POINTER, this->Clone());
        this->Check(pProp);
    }

    void DEM_D_Stress_Dependent_Cohesive::Check(Properties::Pointer pProp) const {
        DEMDiscontinuumConstitutiveLaw::Check(pProp);
        if(!pProp->Has(AMOUNT_OF_COHESION)) {
            KRATOS_WARNING("DEM")<<std::endl;
            KRATOS_WARNING("DEM")<<"WARNING: Variable AMOUNT_OF_COHESION should be present in the properties when using DEM_D_Conical_damage. 90.0 value assigned by default."<<std::endl;
            KRATOS_WARNING("DEM")<<std::endl;
            pProp->GetValue(AMOUNT_OF_COHESION) = 1.0;
        }
    }

    std::string DEM_D_Stress_Dependent_Cohesive::GetTypeOfLaw() {
        std::string type_of_law = "Stress_Dependent_Cohesive";
        return type_of_law;
    }

    /////////////////////////
    // DEM-DEM INTERACTION //
    /////////////////////////

    void DEM_D_Stress_Dependent_Cohesive::InitializeContact(SphericParticle* const element1, SphericParticle* const element2, const double indentation) {
        //Get equivalent Radius
        const double equiv_radius    = 0.5 * (element1->GetRadius() + element2->GetRadius());

        //Get equivalent Young's Modulus
        const double my_young        = element1->GetYoung();
        const double other_young     = element2->GetYoung();
        const double my_poisson      = element1->GetPoisson();
        const double other_poisson   = element2->GetPoisson();
        const double equiv_young     = my_young * other_young / (other_young * (1.0 - my_poisson * my_poisson) + my_young * (1.0 - other_poisson * other_poisson));

        //Get equivalent Shear Modulus
        const double my_shear_modulus = 0.5 * my_young / (1.0 + my_poisson);
        const double other_shear_modulus = 0.5 * other_young / (1.0 + other_poisson);
        const double equiv_shear = 1.0 / ((2.0 - my_poisson)/my_shear_modulus + (2.0 - other_poisson)/other_shear_modulus);

        //Normal and Tangent elastic constants
        mKn = 0.5 * Globals::Pi * equiv_young * equiv_radius;
        mKt = 4.0 * equiv_shear * mKn / equiv_young;
    }

    void DEM_D_Stress_Dependent_Cohesive::CalculateForces(const ProcessInfo& r_process_info,
                                                          const double OldLocalElasticContactForce[3],
                                                          double LocalElasticContactForce[3],
                                                          double LocalDeltDisp[3],
                                                          double LocalRelVel[3],
                                                          double indentation,
                                                          double previous_indentation,
                                                          double ViscoDampingLocalContactForce[3],
                                                          double& cohesive_force,
                                                          SphericParticle* element1,
                                                          SphericParticle* element2,
                                                          bool& sliding,
                                                          double LocalCoordSystem[3][3]) {

        InitializeContact(element1, element2, indentation);

        LocalElasticContactForce[2]  = CalculateNormalForce(indentation);

        DEM_D_Linear_viscous_Coulomb::CalculateViscoDampingForce(LocalRelVel, ViscoDampingLocalContactForce, element1, element2);

        double normal_contact_force = LocalElasticContactForce[2] + ViscoDampingLocalContactForce[2];

        if (normal_contact_force < 0.0) {
            normal_contact_force = 0.0;
            ViscoDampingLocalContactForce[2] = -1.0 * LocalElasticContactForce[2];
        }

        cohesive_force = CalculateCohesiveNormalForce(element1, element2, normal_contact_force, indentation);

        double AuxElasticShearForce;
        double MaximumAdmisibleShearForce;

        CalculateTangentialForceWithNeighbour(normal_contact_force, OldLocalElasticContactForce, LocalElasticContactForce, ViscoDampingLocalContactForce, LocalDeltDisp,
                                              sliding, element1, element2, indentation, previous_indentation, AuxElasticShearForce, MaximumAdmisibleShearForce);

        double& elastic_energy = element1->GetElasticEnergy();
        DEM_D_Linear_viscous_Coulomb::CalculateElasticEnergyDEM(elastic_energy, indentation, LocalElasticContactForce);

        if (AuxElasticShearForce > MaximumAdmisibleShearForce && MaximumAdmisibleShearForce != 0.0){
            double& inelastic_frictional_energy = element1->GetInelasticFrictionalEnergy();
            DEM_D_Linear_viscous_Coulomb::CalculateInelasticFrictionalEnergyDEM(inelastic_frictional_energy, AuxElasticShearForce, LocalElasticContactForce);
        }

        double& inelastic_viscodamping_energy = element1->GetInelasticViscodampingEnergy();
        DEM_D_Linear_viscous_Coulomb::CalculateInelasticViscodampingEnergyDEM(inelastic_viscodamping_energy, ViscoDampingLocalContactForce, LocalDeltDisp);

    }

    /////////////////////////
    // DEM-FEM INTERACTION //
    /////////////////////////

    void DEM_D_Stress_Dependent_Cohesive::InitializeContactWithFEM(SphericParticle* const element, Condition* const wall, const double indentation, const double ini_delta) {
        //Get effective Radius
        const double effective_radius    = element->GetRadius() - ini_delta;

        //Get equivalent Young's Modulus
        const double my_young            = element->GetYoung();
        const double walls_young         = wall->GetProperties()[YOUNG_MODULUS];
        const double my_poisson          = element->GetPoisson();
        const double walls_poisson       = wall->GetProperties()[POISSON_RATIO];
        const double equiv_young         = my_young * walls_young / (walls_young * (1.0 - my_poisson * my_poisson) + my_young * (1.0 - walls_poisson * walls_poisson));

        //Get equivalent Shear Modulus
        const double my_shear_modulus    = 0.5 * my_young / (1.0 + my_poisson);
        const double walls_shear_modulus = 0.5 * walls_young / (1.0 + walls_poisson);
        const double equiv_shear         = 1.0 / ((2.0 - my_poisson)/my_shear_modulus + (2.0 - walls_poisson)/walls_shear_modulus);

        mKn = 0.5 * Globals::Pi * equiv_young * effective_radius;
        mKt = 4.0 * equiv_shear * mKn / equiv_young;
    }

    void DEM_D_Stress_Dependent_Cohesive::CalculateForcesWithFEM(ProcessInfo& r_process_info,
                                                                 const double OldLocalElasticContactForce[3],
                                                                 double LocalElasticContactForce[3],
                                                                 double LocalDeltDisp[3],
                                                                 double LocalRelVel[3],
                                                                 double indentation,
                                                                 double previous_indentation,
                                                                 double ViscoDampingLocalContactForce[3],
                                                                 double& cohesive_force,
                                                                 SphericParticle* const element,
                                                                 Condition* const wall,
                                                                 bool& sliding) {

        InitializeContactWithFEM(element, wall, indentation);

        LocalElasticContactForce[2] = CalculateNormalForce(element, wall, indentation);

        DEM_D_Linear_viscous_Coulomb::CalculateViscoDampingForceWithFEM(LocalRelVel, ViscoDampingLocalContactForce, element, wall);

        double normal_contact_force = LocalElasticContactForce[2] + ViscoDampingLocalContactForce[2];

        if (normal_contact_force < 0.0) {
            normal_contact_force = 0.0;
            ViscoDampingLocalContactForce[2] = -1.0 * LocalElasticContactForce[2];
        }

        cohesive_force = CalculateCohesiveNormalForceWithFEM(element, wall, normal_contact_force, indentation);

        double AuxElasticShearForce;
        double MaximumAdmisibleShearForce;

        CalculateTangentialForceWithNeighbour(normal_contact_force, OldLocalElasticContactForce, LocalElasticContactForce, ViscoDampingLocalContactForce, LocalDeltDisp,
                                              sliding, element, wall, indentation, previous_indentation, AuxElasticShearForce, MaximumAdmisibleShearForce);

        double& elastic_energy = element->GetElasticEnergy();
        DEM_D_Linear_viscous_Coulomb::CalculateElasticEnergyFEM(elastic_energy, indentation, LocalElasticContactForce);//MSIMSI

        if (AuxElasticShearForce > MaximumAdmisibleShearForce && MaximumAdmisibleShearForce != 0.0){
            double& inelastic_frictional_energy = element->GetInelasticFrictionalEnergy();
            DEM_D_Linear_viscous_Coulomb::CalculateInelasticFrictionalEnergyFEM(inelastic_frictional_energy, AuxElasticShearForce, LocalElasticContactForce);
        }

        double& inelastic_viscodamping_energy = element->GetInelasticViscodampingEnergy();
        DEM_D_Linear_viscous_Coulomb::CalculateInelasticViscodampingEnergyFEM(inelastic_viscodamping_energy, ViscoDampingLocalContactForce, LocalDeltDisp);
    }

    template<class NeighbourClassType>

    void DEM_D_Stress_Dependent_Cohesive::CalculateTangentialForceWithNeighbour(const double normal_contact_force,
                                                                                const double OldLocalElasticContactForce[3],
                                                                                double LocalElasticContactForce[3],
                                                                                double ViscoDampingLocalContactForce[3],
                                                                                const double LocalDeltDisp[3],
                                                                                bool& sliding,
                                                                                SphericParticle* const element,
                                                                                NeighbourClassType* const neighbour,
                                                                                double indentation,
                                                                                double previous_indentation,
                                                                                double& AuxElasticShearForce,
                                                                                double& MaximumAdmisibleShearForce) {

        LocalElasticContactForce[0] = OldLocalElasticContactForce[0] - mKt * LocalDeltDisp[0];
        LocalElasticContactForce[1] = OldLocalElasticContactForce[1] - mKt * LocalDeltDisp[1];

        AuxElasticShearForce = sqrt(LocalElasticContactForce[0] * LocalElasticContactForce[0] + LocalElasticContactForce[1] * LocalElasticContactForce[1]);

        const double my_tg_of_friction_angle    = element->GetTgOfFrictionAngle();
        const double wall_tg_of_friction_angle  = neighbour->GetProperties()[FRICTION];
        const double equiv_tg_of_fri_ang        = 0.5 * (my_tg_of_friction_angle + wall_tg_of_friction_angle);

        MaximumAdmisibleShearForce = normal_contact_force * equiv_tg_of_fri_ang;

        const double tangential_contact_force_0 = LocalElasticContactForce[0] + ViscoDampingLocalContactForce[0];
        const double tangential_contact_force_1 = LocalElasticContactForce[1] + ViscoDampingLocalContactForce[1];

        const double ActualTotalShearForce = sqrt(tangential_contact_force_0 * tangential_contact_force_0 + tangential_contact_force_1 * tangential_contact_force_1);

        if (ActualTotalShearForce > MaximumAdmisibleShearForce) {

            const double ActualElasticShearForce = sqrt(LocalElasticContactForce[0] * LocalElasticContactForce[0] + LocalElasticContactForce[1] * LocalElasticContactForce[1]);

            const double dot_product = LocalElasticContactForce[0] * ViscoDampingLocalContactForce[0] + LocalElasticContactForce[1] * ViscoDampingLocalContactForce[1];
            const double ViscoDampingLocalContactForceModule = sqrt(ViscoDampingLocalContactForce[0] * ViscoDampingLocalContactForce[0] +\
                                                                    ViscoDampingLocalContactForce[1] * ViscoDampingLocalContactForce[1]);

            if (dot_product >= 0.0) {

                if (ActualElasticShearForce > MaximumAdmisibleShearForce) {
                    const double fraction = MaximumAdmisibleShearForce / ActualElasticShearForce;
                    LocalElasticContactForce[0]      = LocalElasticContactForce[0] * fraction;
                    LocalElasticContactForce[1]      = LocalElasticContactForce[1] * fraction;
                    ViscoDampingLocalContactForce[0] = 0.0;
                    ViscoDampingLocalContactForce[1] = 0.0;
                }
                else {
                    const double ActualViscousShearForce = MaximumAdmisibleShearForce - ActualElasticShearForce;
                    const double fraction = ActualViscousShearForce / ViscoDampingLocalContactForceModule;
                    ViscoDampingLocalContactForce[0] *= fraction;
                    ViscoDampingLocalContactForce[1] *= fraction;
                }
            }
            else {
                if (ViscoDampingLocalContactForceModule >= ActualElasticShearForce) {
                    const double fraction = (MaximumAdmisibleShearForce + ActualElasticShearForce) / ViscoDampingLocalContactForceModule;
                    ViscoDampingLocalContactForce[0] *= fraction;
                    ViscoDampingLocalContactForce[1] *= fraction;
                }
                else {
                    const double fraction = MaximumAdmisibleShearForce / ActualElasticShearForce;
                    LocalElasticContactForce[0]      = LocalElasticContactForce[0] * fraction;
                    LocalElasticContactForce[1]      = LocalElasticContactForce[1] * fraction;
                    ViscoDampingLocalContactForce[0] = 0.0;
                    ViscoDampingLocalContactForce[1] = 0.0;
                }
            }
            sliding = true;
        }
    }

    double DEM_D_Stress_Dependent_Cohesive::CalculateNormalForce(const double indentation) {
        return mKn * indentation;
    }

    double DEM_D_Stress_Dependent_Cohesive::CalculateCohesiveNormalForce(SphericParticle* const element1, SphericParticle* const element2, const double normal_contact_force, const double indentation) {

        double equiv_cohesion       = 0.0;
        double equiv_amount_of_cohesion = 0.5 * (element1->GetAmountOfCohesion() + element2->GetAmountOfCohesion());

        const double equiv_radius   = 0.5 * (element1->GetRadius() + element2->GetRadius());

        for (unsigned int i = 0; element1->mNeighbourElements.size(); i++) {
            if (element1->mNeighbourElements[i]->Id() == element2->Id()) {
                equiv_cohesion = std::min(0.5 * (element1->GetParticleCohesion() + element2->GetParticleCohesion()), equiv_amount_of_cohesion * element1->mNeighbourContactStress[i]);
                double contact_stress = normal_contact_force / (Globals::Pi * equiv_radius * equiv_radius);
                element1->mNeighbourContactStress[i] = std::max(element1->mNeighbourContactStress[i], contact_stress);
                break;
            }
        }

        const double cohesive_force = Globals::Pi * equiv_cohesion * equiv_radius* equiv_radius;

        return cohesive_force;
    }

    double DEM_D_Stress_Dependent_Cohesive::CalculateCohesiveNormalForceWithFEM(SphericParticle* const element, Condition* const wall, const double normal_contact_force, const double indentation) {

        double equiv_cohesion = 0.0;
        double equiv_amount_of_cohesion = element->GetAmountOfCohesion();

        const double equiv_radius     = element->GetRadius(); // Equivalent Radius for RIGID WALLS

        for (unsigned int i = 0; element->mNeighbourRigidFaces.size(); i++) {
            if (element->mNeighbourRigidFaces[i]->Id() == wall->Id()) {
                equiv_cohesion = std::min(0.5 * (element->GetParticleCohesion() + wall->GetProperties()[WALL_COHESION]), equiv_amount_of_cohesion * element->mNeighbourRigidContactStress[i]);
                double contact_stress = normal_contact_force / (Globals::Pi * equiv_radius * equiv_radius);
                element->mNeighbourRigidContactStress[i] = std::max(element->mNeighbourRigidContactStress[i], contact_stress);
                break;
            }
        }

        const double cohesive_force   = Globals::Pi * equiv_cohesion * equiv_radius * equiv_radius;

        return cohesive_force;
    }

} // namespace Kratos
