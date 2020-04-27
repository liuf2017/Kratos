# -*- coding: utf-8 -*-
import itertools, sys, re 
import numpy as np 
sys.path.append("/work/piquee/Softwares/TAU/TAU_2016.2/2016.2.0/bin/py_turb1eq")
import tau_python 
from tau_python import tau_msg

def findFileName0(list_of_interface_file_paths,working_path,word): 
    for file in list_of_interface_file_paths:
        if file.startswith('%s'%working_path + '%s'%word): ####### i would like to make it general #######
            print file
            return file

def findFileName(list_of_interface_file_paths,working_path,word,this_step_out): 
   # this_step_out +=1
    for file in list_of_interface_file_paths:
        if file.startswith('%s'%working_path + '%s'%word + '%s'%this_step_out): ####### i would like to make it general #######
            print file
            return file

def findInterfaceFileNumberOfLines(fname):
    with open(fname,'r') as f:
        it = 0
        pattern = {'E+', 'E-'}
        for line in f:
            if 'E+' in line or 'E-' in line:
                it = it+1
    return it

def PrintBlockHeader(header):
 	tau_python.tau_msg("\n" + 50 * "*" + "\n" + "* %s\n" %header + 50*"*" + "\n")

def readTautoplt(fname_mod, fname_o, mesh_iteration, interface_file_name, para_path_mod):
    fs = open(fname_o,'r+')
    fd = open(fname_mod,'w')
    line = fs.readline()
    while line:
        if 'Primary grid filename:' in line:
	    line = 'Primary grid filename:' + mesh_iteration + ' \n'
	    fd.write(line) 
            print line
            line = fs.readline()
        if 'Boundary mapping filename:' in line:
	    line = 'Boundary mapping filename:' + para_path_mod + ' \n'
	    fd.write(line)  
            print line
            line = fs.readline()
        if 'Restart-data prefix:' in line:
            line = 'Restart-data prefix:' + interface_file_name + ' \n'
            fd.write(line)
            print line
            line = fs.readline()
        else:
            line = fs.readline()
            fd.write(line)
    fd.close()
    fs.close()


# Read Cp from the solution file and calculate 'Pressure' on the nodes of TAU Mesh
def readPressure(interface_file_name,interface_file_number_of_lines,error,velocity):
    with open(interface_file_name,'r') as f:
        header1 = f.readline()  #liest document linie für linie durch 
        header2 = f.readline() 
        print "Careful ---- headers 2 = ", header2 #5 readline- Annahme fünf spalten 
        header2_split = header2.split()
        pos_X = header2_split.index('"x"')
        pos_Y = header2_split.index('"y"')
        pos_Z = header2_split.index('"z"')
        pos_Cp = header2_split.index('"cp"')
        header3 = f.readline()
        header4 = f.readline()
        header5 = f.readline()

        d=[int(s) for s in re.findall(r'\b\d+\b', header4)]  #find all sucht muster im document 
        NodesNr = d[0]
        ElemsNr = d[1]

        # write X,Y,Z,CP of the document in a vector = liste_number
        liste_number = []
        for i in xrange(interface_file_number_of_lines):
           line = f.readline()
           for elem in line.split():
               liste_number.append(float(elem))
	
        # write ElemTable of the document 
        elemTable_Sol = np.zeros([ElemsNr,4],dtype=int)
        k = 0
        line = f.readline()
        while line:
            elemTable_Sol[k,0]=int(line.split()[0]) 
            elemTable_Sol[k,1]=int(line.split()[1])
            elemTable_Sol[k,2]=int(line.split()[2])
            elemTable_Sol[k,3]=int(line.split()[3])
            k=k+1
            line = f.readline()

        # reshape content in X, Y, Z, Cp
        X=liste_number[(pos_X-2)*NodesNr:(pos_X-2+1)*NodesNr]
        Y=liste_number[(pos_Y-2)*NodesNr:(pos_Y-2+1)*NodesNr]
        Z=liste_number[(pos_Z-2)*NodesNr:(pos_Z-2+1)*NodesNr]
        CP=liste_number[(pos_Cp-2)*NodesNr:(pos_Cp-2+1)*NodesNr]
        X=X[0:NodesNr-error]
        Y=Y[0:NodesNr-error]
        Z=Z[0:NodesNr-error]
        CP=CP[0:NodesNr-error]
        P=[x*velocity*velocity*0.5*1.2 for x in CP]
        P=np.array(P)

    return NodesNr,ElemsNr,X,Y,Z,CP,P,elemTable_Sol,liste_number


def interfaceMeshFluid(NodesNr, ElemsNr, elemTable, X, Y, Z):
    # array to store the coordinates of the nodes in the fluid mesh: x1,y1,z1,x2,y2,z2,...
    nodes = np.zeros(NodesNr*3)
    # array to store the IDs of the nodes in the fluid mesh: IDnode1, IDnode2,...
    nodesID = np.zeros(NodesNr, dtype=int)
    elems = np.zeros(4*ElemsNr, dtype=int)  # array to store the element table
    element_types = np.zeros(ElemsNr, dtype=int)

    for i in xrange(0, NodesNr):
        nodesID[i] = i+1
        nodes[3*i+0] = X[i]
        nodes[3*i+1] = Y[i]
        nodes[3*i+2] = Z[i]

    for i in xrange(0, ElemsNr):
        elems[i*4+0] = elemTable[i, 0]
        elems[i*4+1] = elemTable[i, 1]
        elems[i*4+2] = elemTable[i, 2]
        elems[i*4+3] = elemTable[i, 3]
        element_types[i] = 9

    return nodes, nodesID, elems, element_types

# Calculate the Pressure on the elements from the pressure on the nodes
def calcpCell(ElemsNr,P,X,elemTable):
    pCell = np.zeros(ElemsNr); # cp for interface elements
    #print 'len(elemTable) = ', len(elemTable)
    with open('xp','w') as f:
        for i in xrange(0,ElemsNr): 
            pCell[i] = 0.25* (P[elemTable[i,0]-1] + P[elemTable[i,1]-1] + P[elemTable[i,2]-1] + P[elemTable[i,3]-1]);
            x= 0.25* (X[elemTable[i,0]-1] + X[elemTable[i,1]-1] + X[elemTable[i,2]-1] + X[elemTable[i,3]-1]);
            f.write('%d\t%f\t%f\n'%(i,x,pCell[i]))
    
    return pCell


# Calculate the area and the normal of the area for each cell - element of TAU Mesh
def calcAreaNormal(ElemsNr,elemTable,X,Y,Z,fIteration):
    area = np.zeros(ElemsNr); # area of each interface element
    normal = np.zeros([ElemsNr,3]) # element normal vector
    A = np.zeros(3);
    B = np.zeros(3);
    for i in xrange(0,ElemsNr):
        A[0] = X[elemTable[i,2]-1] - X[elemTable[i,0]-1]
        A[1] = Y[elemTable[i,2]-1] - Y[elemTable[i,0]-1]
        A[2] = Z[elemTable[i,2]-1] - Z[elemTable[i,0]-1]
        B[0] = X[elemTable[i,3]-1] - X[elemTable[i,1]-1]
        B[1] = Y[elemTable[i,3]-1] - Y[elemTable[i,1]-1]
        B[2] = Z[elemTable[i,3]-1] - Z[elemTable[i,1]-1]
        a=np.cross(A,B)   #np quasi ein matematischer provider
        norm=np.linalg.norm(a)
        a=a/norm
        normal[i,0]=a[0]
        normal[i,1]=a[1]
        normal[i,2]=a[2]
        A[0] = X[elemTable[i,1]-1] - X[elemTable[i,0]-1]
        A[1] = Y[elemTable[i,1]-1] - Y[elemTable[i,0]-1]
        A[2] = Z[elemTable[i,1]-1] - Z[elemTable[i,0]-1]
        B[0] = X[elemTable[i,3]-1] - X[elemTable[i,0]-1]
        B[1] = Y[elemTable[i,3]-1] - Y[elemTable[i,0]-1]
        B[2] = Z[elemTable[i,3]-1] - Z[elemTable[i,0]-1]
        a=np.cross(A,B)
        norm=np.linalg.norm(a)
        area[i] =  norm # hold only for 2d case, MUST be modified for 3d interfaces
    f_name = 'Outputs/Area_'+str(fIteration)+'.dat'
    with open(f_name,'w') as fwrite:
        for i in xrange(0,len(area[:])):
            fwrite.write("%f\n" % (area[i]))
    return area, normal

		    
# Calculate the Vector Force 
def calcFluidForceVector(ElemsNr,elemTable,NodesNr,pCell,area,normal,fIteration):
    forcesTauNP = np.zeros(NodesNr*3)
    for i in xrange(0,ElemsNr):
        #p= cpCell[i] * q
        p=pCell[i]
        Fx = p * area[i] * normal[i,0]
        Fy = p * area[i] * normal[i,1]
        Fz = p * area[i] * normal[i,2]
        #print 'test Fx, Fy, Fz', Fx, Fy, Fz
        forcesTauNP[3*(elemTable[i,0]-1)+0] += 0.25 * Fx
        forcesTauNP[3*(elemTable[i,0]-1)+1] += 0.25 * Fy
        forcesTauNP[3*(elemTable[i,0]-1)+2] += 0.25 * Fz
        forcesTauNP[3*(elemTable[i,1]-1)+0] += 0.25 * Fx
        forcesTauNP[3*(elemTable[i,1]-1)+1] += 0.25 * Fy
        forcesTauNP[3*(elemTable[i,1]-1)+2] += 0.25 * Fz
        forcesTauNP[3*(elemTable[i,2]-1)+0] += 0.25 * Fx
        forcesTauNP[3*(elemTable[i,2]-1)+1] += 0.25 * Fy
        forcesTauNP[3*(elemTable[i,2]-1)+2] += 0.25 * Fz
        forcesTauNP[3*(elemTable[i,3]-1)+0] += 0.25 * Fx
        forcesTauNP[3*(elemTable[i,3]-1)+1] += 0.25 * Fy
        forcesTauNP[3*(elemTable[i,3]-1)+2] += 0.25 * Fz
    f_name = 'Outputs/ForcesTauNP_'+str(fIteration)+'.dat'
    with open(f_name,'w') as fwrite:
        for i in xrange(0,len(forcesTauNP[:])):
            fwrite.write("%f\n" % (forcesTauNP[i]))
    return forcesTauNP